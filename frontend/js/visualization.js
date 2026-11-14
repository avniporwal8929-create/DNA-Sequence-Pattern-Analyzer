/**
 * Dynamic SVG Automata Visualizer
 * Renders NFAs and DFAs returned by the C++ engine.
 * Supports zoom, pan, active state highlighting, edge curvature, and SVG export.
 */

class AutomataVisualizer {
  constructor(containerId, options = {}) {
    this.container = document.getElementById(containerId);
    this.options = Object.assign({
      nodeRadius: 24,
      layerSpacingX: 130,
      layerSpacingY: 100,
      onNodeSelect: null
    }, options);

    this.zoom = 1.0;
    this.panX = 0;
    this.panY = 0;
    this.isPanning = false;
    this.startX = 0;
    this.startY = 0;

    this.currentAutomaton = null;
    this.activeStateId = null;
    this.activeEdge = null;

    this.init();
  }

  init() {
    if (!this.container) return;
    this.container.innerHTML = `
      <div class="automata-toolbar">
        <div style="display:flex; align-items:center; gap:8px;">
          <span style="font-size:12px; font-weight:700; color:var(--color-text-main);" id="${this.container.id}-title">Automaton Graph</span>
          <span class="card-badge" id="${this.container.id}-badge">Ready</span>
        </div>
        <div class="btn-group">
          <button class="btn btn-outline btn-sm" title="Zoom In" id="${this.container.id}-zoom-in">+</button>
          <button class="btn btn-outline btn-sm" title="Zoom Out" id="${this.container.id}-zoom-out">-</button>
          <button class="btn btn-outline btn-sm" title="Reset View" id="${this.container.id}-reset-view">Reset</button>
          <button class="btn btn-outline btn-sm" title="Export as SVG" id="${this.container.id}-export-svg">Export SVG</button>
        </div>
      </div>
      <div class="automata-canvas-area" id="${this.container.id}-canvas">
        <svg class="automata-svg" id="${this.container.id}-svg" xmlns="http://www.w3.org/2000/svg">
          <defs>
            <marker id="arrow-normal" viewBox="0 0 10 10" refX="24" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
              <path d="M 0 1.5 L 10 5 L 0 8.5 z" fill="#48594F" />
            </marker>
            <marker id="arrow-active" viewBox="0 0 10 10" refX="24" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse">
              <path d="M 0 1.5 L 10 5 L 0 8.5 z" fill="#D9383A" />
            </marker>
          </defs>
          <g id="${this.container.id}-viewport"></g>
        </svg>
      </div>
    `;

    this.svg = document.getElementById(`${this.container.id}-svg`);
    this.viewport = document.getElementById(`${this.container.id}-viewport`);
    this.canvasArea = document.getElementById(`${this.container.id}-canvas`);

    // Attach zoom & pan event listeners
    document.getElementById(`${this.container.id}-zoom-in`).addEventListener('click', () => this.zoomBy(1.2));
    document.getElementById(`${this.container.id}-zoom-out`).addEventListener('click', () => this.zoomBy(0.8));
    document.getElementById(`${this.container.id}-reset-view`).addEventListener('click', () => this.resetView());
    document.getElementById(`${this.container.id}-export-svg`).addEventListener('click', () => this.exportSVG());

    this.setupPanAndZoom();
  }

  setupPanAndZoom() {
    this.canvasArea.addEventListener('mousedown', (e) => {
      if (e.target.closest('.svg-state-group')) return; // Allow node clicking
      this.isPanning = true;
      this.startX = e.clientX - this.panX;
      this.startY = e.clientY - this.panY;
    });

    window.addEventListener('mousemove', (e) => {
      if (!this.isPanning) return;
      this.panX = e.clientX - this.startX;
      this.panY = e.clientY - this.startY;
      this.updateTransform();
    });

    window.addEventListener('mouseup', () => {
      this.isPanning = false;
    });

    this.canvasArea.addEventListener('wheel', (e) => {
      e.preventDefault();
      const zoomFactor = e.deltaY < 0 ? 1.1 : 0.9;
      this.zoomBy(zoomFactor);
    }, { passive: false });
  }

  zoomBy(factor) {
    this.zoom = Math.min(Math.max(this.zoom * factor, 0.3), 3.0);
    this.updateTransform();
  }

  resetView() {
    this.zoom = 1.0;
    this.panX = 40;
    this.panY = 40;
    this.updateTransform();
  }

  updateTransform() {
    if (this.viewport) {
      this.viewport.setAttribute('transform', `translate(${this.panX}, ${this.panY}) scale(${this.zoom})`);
    }
  }

  // Calculate layout coordinates using BFS layering from start state
  computeLayout(automaton) {
    const states = automaton.states || [];
    const edges = automaton.edges || [];
    const startStateId = automaton.startState;

    const adj = new Map();
    states.forEach(s => adj.set(s.id, []));
    edges.forEach(e => {
      if (adj.has(e.from)) adj.get(e.from).push(e.to);
    });

    // BFS for layers
    const layer = new Map();
    const queue = [startStateId];
    layer.set(startStateId, 0);

    while (queue.length > 0) {
      const u = queue.shift();
      const currL = layer.get(u);
      const neighbors = adj.get(u) || [];
      for (const v of neighbors) {
        if (!layer.has(v)) {
          layer.set(v, currL + 1);
          queue.push(v);
        }
      }
    }

    // Assign any unvisited states to layer 0 or max layer + 1
    let maxL = 0;
    layer.forEach(l => { if (l > maxL) maxL = l; });
    states.forEach(s => {
      if (!layer.has(s.id)) layer.set(s.id, maxL + 1);
    });

    // Group states by layer
    const layers = new Map();
    states.forEach(s => {
      const l = layer.get(s.id);
      if (!layers.has(l)) layers.set(l, []);
      layers.get(l).push(s);
    });

    const positions = new Map();
    const startX = 70;
    const centerY = 240;

    layers.forEach((stateList, l) => {
      const x = startX + l * this.options.layerSpacingX;
      const count = stateList.length;
      stateList.forEach((s, idx) => {
        const offset = (idx - (count - 1) / 2) * this.options.layerSpacingY;
        const y = centerY + offset;
        positions.set(s.id, { x, y });
      });
    });

    return positions;
  }

  render(automaton, title = "Automaton Graph") {
    this.currentAutomaton = automaton;
    const titleEl = document.getElementById(`${this.container.id}-title`);
    const badgeEl = document.getElementById(`${this.container.id}-badge`);
    if (titleEl) titleEl.textContent = title;
    if (badgeEl) {
      const type = automaton.type || "Automaton";
      const stateCount = automaton.states ? automaton.states.length : 0;
      badgeEl.textContent = `${type} (${stateCount} states)`;
    }

    if (!automaton || !automaton.states || automaton.states.length === 0) {
      this.viewport.innerHTML = `<text x="50" y="50" fill="#788078" font-size="14">No automaton loaded.</text>`;
      return;
    }

    const positions = this.computeLayout(automaton);
    let svgContent = '';

    // 1. Render Initial State Arrow
    const startPos = positions.get(automaton.startState);
    if (startPos) {
      const arrowStartX = startPos.x - 45;
      const arrowStartY = startPos.y;
      svgContent += `
        <g class="svg-initial-arrow">
          <path d="M ${arrowStartX} ${arrowStartY} L ${startPos.x - this.options.nodeRadius} ${arrowStartY}" 
                stroke="#17211B" stroke-width="2.2" marker-end="url(#arrow-normal)" />
          <text x="${arrowStartX - 8}" y="${arrowStartY + 4}" font-family="var(--font-mono)" font-size="11" font-weight="700" fill="#17211B" text-anchor="end">START</text>
        </g>
      `;
    }

    // 2. Render Transitions / Edges
    const edges = automaton.edges || [];
    // Group multiple symbols between same from -> to
    const groupedEdges = new Map();
    edges.forEach(e => {
      const key = `${e.from}->${e.to}`;
      if (!groupedEdges.has(key)) {
        groupedEdges.set(key, { from: e.from, to: e.to, symbols: [e.symbol], isEpsilon: e.isEpsilon });
      } else {
        if (!groupedEdges.get(key).symbols.includes(e.symbol)) {
          groupedEdges.get(key).symbols.push(e.symbol);
        }
      }
    });

    groupedEdges.forEach(edge => {
      const p1 = positions.get(edge.from);
      const p2 = positions.get(edge.to);
      if (!p1 || !p2) return;

      const symLabel = edge.symbols.join(',');
      const isLoop = (edge.from === edge.to);

      if (isLoop) {
        // Self-loop on top of node
        const r = this.options.nodeRadius;
        const loopR = 18;
        const x1 = p1.x - 10;
        const y1 = p1.y - r;
        const x2 = p1.x + 10;
        const y2 = p1.y - r;
        const pathD = `M ${x1} ${y1} C ${p1.x - 28} ${p1.y - r - 45}, ${p1.x + 28} ${p1.y - r - 45}, ${x2} ${y2}`;

        svgContent += `
          <g class="svg-edge-group" data-from="${edge.from}" data-to="${edge.to}">
            <path d="${pathD}" class="svg-edge-path" id="edge-${edge.from}-${edge.to}" />
            <rect x="${p1.x - 14}" y="${p1.y - r - 38}" width="28" height="15" rx="3" class="svg-edge-label-bg" />
            <text x="${p1.x}" y="${p1.y - r - 27}" class="svg-edge-label">${symLabel}</text>
          </g>
        `;
      } else {
        // Check if reciprocal edge exists (p2 -> p1) to introduce curvature
        const reciprocalKey = `${edge.to}->${edge.from}`;
        const hasReciprocal = groupedEdges.has(reciprocalKey);

        const dx = p2.x - p1.x;
        const dy = p2.y - p1.y;
        const dist = Math.sqrt(dx * dx + dy * dy);

        // Control point for curve if needed
        let curveX, curveY;
        if (hasReciprocal || Math.abs(dy) > 10) {
          const normalX = -dy / dist;
          const normalY = dx / dist;
          const bend = hasReciprocal ? 35 : 15;
          const midX = (p1.x + p2.x) / 2;
          const midY = (p1.y + p2.y) / 2;
          curveX = midX + normalX * bend;
          curveY = midY + normalY * bend;
        } else {
          curveX = (p1.x + p2.x) / 2;
          curveY = (p1.y + p2.y) / 2;
        }

        const pathD = (hasReciprocal || Math.abs(dy) > 10)
          ? `M ${p1.x} ${p1.y} Q ${curveX} ${curveY} ${p2.x} ${p2.y}`
          : `M ${p1.x} ${p1.y} L ${p2.x} ${p2.y}`;

        svgContent += `
          <g class="svg-edge-group" data-from="${edge.from}" data-to="${edge.to}">
            <path d="${pathD}" class="svg-edge-path" id="edge-${edge.from}-${edge.to}" />
            <rect x="${curveX - 14}" y="${curveY - 9}" width="28" height="18" rx="3" class="svg-edge-label-bg" />
            <text x="${curveX}" y="${curveY + 4}" class="svg-edge-label">${symLabel}</text>
          </g>
        `;
      }
    });

    // 3. Render States / Nodes
    automaton.states.forEach(state => {
      const pos = positions.get(state.id);
      if (!pos) return;

      const r = this.options.nodeRadius;
      const isAccept = state.isAccept;
      const isStart = state.isStart;
      const label = state.label || `q${state.id}`;
      const subset = state.subset || "";

      svgContent += `
        <g class="svg-state-group" id="node-${state.id}" data-id="${state.id}" data-label="${label}" data-subset="${subset}" style="cursor:pointer;">
          <circle cx="${pos.x}" cy="${pos.y}" r="${r}" class="svg-state-circle ${isAccept ? 'accept-outer' : ''}" />
          ${isAccept ? `<circle cx="${pos.x}" cy="${pos.y}" r="${r - 4.5}" class="svg-state-circle accept-inner" />` : ''}
          <text x="${pos.x}" y="${pos.y + 1}" class="svg-state-text">${label}</text>
          ${subset ? `<text x="${pos.x}" y="${pos.y + r + 15}" font-family="var(--font-mono)" font-size="10.5" fill="#556B5F" text-anchor="middle">${subset}</text>` : ''}
        </g>
      `;
    });

    this.viewport.innerHTML = svgContent;
    this.bindNodeEvents();
    this.resetView();
  }

  bindNodeEvents() {
    const nodes = this.viewport.querySelectorAll('.svg-state-group');
    nodes.forEach(node => {
      node.addEventListener('click', () => {
        const stateId = parseInt(node.getAttribute('data-id'), 10);
        const label = node.getAttribute('data-label');
        const subset = node.getAttribute('data-subset');
        this.highlightState(stateId);
        if (typeof this.options.onNodeSelect === 'function') {
          this.options.onNodeSelect({ id: stateId, label, subset });
        }
      });
    });
  }

  highlightState(stateId) {
    this.activeStateId = stateId;
    const circles = this.viewport.querySelectorAll('.svg-state-circle');
    circles.forEach(c => c.classList.remove('active'));

    const activeGroup = this.viewport.querySelector(`#node-${stateId}`);
    if (activeGroup) {
      activeGroup.querySelectorAll('.svg-state-circle').forEach(c => c.classList.add('active'));
    }
  }

  highlightTransition(fromId, toId) {
    const edges = this.viewport.querySelectorAll('.svg-edge-path');
    edges.forEach(e => e.classList.remove('active'));

    const activeEdge = this.viewport.querySelector(`#edge-${fromId}-${toId}`);
    if (activeEdge) {
      activeEdge.classList.add('active');
    }
  }

  clearHighlight() {
    this.activeStateId = null;
    this.viewport.querySelectorAll('.svg-state-circle').forEach(c => c.classList.remove('active'));
    this.viewport.querySelectorAll('.svg-edge-path').forEach(e => e.classList.remove('active'));
  }

  exportSVG() {
    if (!this.svg) return;
    const svgData = new XMLSerializer().serializeToString(this.svg);
    const blob = new Blob([svgData], { type: 'image/svg+xml;charset=utf-8' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = `automaton-${Date.now()}.svg`;
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);
  }
}
