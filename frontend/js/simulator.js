/**
 * Interactive DFA Simulator Controller
 * Updated for GENOME·AUTOMATA dark biotech UI
 */

const SimulatorModule = {
  visualizer: null,
  currentDFA: null,
  currentPatternName: '',
  trace: [],
  currentStepIndex: 0,

  init() {
    this.visualizer = new AutomataVisualizer('sim-dfa-container');

    document.getElementById('sim-run-btn')?.addEventListener('click', () => this.startSimulation());
    document.getElementById('sim-clear-btn')?.addEventListener('click', () => this.clearSimulation());
  },

  loadDFA(dfa, patternName = '') {
    this.currentDFA = dfa;
    this.currentPatternName = patternName;
    if (this.visualizer && dfa) {
      this.visualizer.render(dfa, `DFA: ${patternName || 'Loaded Automaton'}`);
    }
  },

  clearSimulation() {
    document.getElementById('sim-tape').innerHTML = '';
    document.getElementById('sim-verdict').innerHTML = '';
    document.getElementById('sim-trace').innerHTML =
      `<div style="text-align:center;padding:24px;color:var(--text-muted);font-size:12px;">Run a simulation to see the execution trace.</div>`;
    document.getElementById('sim-state-display').innerHTML =
      `<span style="color:var(--text-muted);font-size:12px;">Run a simulation to see state information.</span>`;
    const badge = document.getElementById('sim-verdict-badge');
    if (badge) { badge.textContent = '—'; badge.className = 'card-badge'; }
  },

  renderTape(seq, activeIdx) {
    const tape = document.getElementById('sim-tape');
    if (!tape) return;
    tape.innerHTML = seq.split('').map((c, i) => {
      const cls = i < activeIdx ? 'tape-cell done' : i === activeIdx ? 'tape-cell active' : 'tape-cell';
      return `<div class="${cls}">${c}</div>`;
    }).join('');
  },

  async startSimulation() {
    const seq    = document.getElementById('sim-seq-input')?.value.trim() || '';
    const regex  = document.getElementById('sim-regex-source-input')?.value.trim() || 'ATG';
    const verdict = document.getElementById('sim-verdict');
    const trace   = document.getElementById('sim-trace');
    const badge   = document.getElementById('sim-verdict-badge');
    const stateDisplay = document.getElementById('sim-state-display');

    if (!seq) { App.showToast('Enter a DNA sequence to simulate.'); return; }

    if (trace) trace.innerHTML = `<div style="display:flex;align-items:center;gap:8px;padding:16px;color:var(--text-muted);font-size:12px;"><div class="spinner"></div> Simulating DFA…</div>`;

    const res = await API.simulateDFA(regex, seq, 'regex');
    if (!res.success) { App.showToast(res.errorMessage || 'Simulation failed'); return; }

    this.currentDFA = res.dfa;
    this.trace = res.trace || [];

    // Render the DFA graph
    if (this.visualizer && res.dfa) {
      this.visualizer.render(res.dfa, `DFA: ${regex}`);
    }

    // Render tape (fully done since full run)
    this.renderTape(seq, seq.length);

    // Verdict banner
    if (verdict) {
      if (res.isAccepted) {
        verdict.innerHTML = `<div class="verdict-accepted">✓ ACCEPTED — Sequence reaches an accepting state.</div>`;
      } else {
        verdict.innerHTML = `<div class="verdict-rejected">✗ REJECTED — Sequence halts in a non-accepting state.</div>`;
      }
    }

    if (badge) {
      badge.textContent = res.isAccepted ? 'ACCEPTED' : 'REJECTED';
      badge.className = `card-badge ${res.isAccepted ? 'emerald' : 'danger'}`;
    }

    // State display
    if (stateDisplay && this.trace.length > 0) {
      const last = this.trace[this.trace.length - 1];
      stateDisplay.innerHTML = `
        <div class="info-row">
          <span class="info-label">Final State</span>
          <span class="info-value text-${res.isAccepted ? 'emerald' : 'danger'} text-mono">${last.nextStateLabel || last.currentStateLabel}</span>
        </div>
        <div class="info-row">
          <span class="info-label">Total Steps</span>
          <span class="info-value">${this.trace.length}</span>
        </div>
        <div class="info-row" style="border-bottom:none;">
          <span class="info-label">Result</span>
          <span class="info-value text-${res.isAccepted ? 'emerald' : 'danger'} font-bold">${res.isAccepted ? 'STRING ACCEPTED' : 'STRING REJECTED'}</span>
        </div>`;
    }

    // Trace log
    if (trace && this.trace.length > 0) {
      trace.innerHTML = this.trace.map((step, idx) => `
        <div class="trace-row ${idx === this.trace.length - 1 ? 'current' : ''}">
          <span class="trace-step">${step.stepNumber}</span>
          <span class="trace-state">${step.currentStateLabel}</span>
          <span class="trace-arrow">─</span>
          <span class="trace-sym">${step.inputSymbol}</span>
          <span class="trace-arrow">→</span>
          <span class="trace-next">${step.nextStateLabel}</span>
          <span style="flex:1;font-size:10.5px;color:var(--text-muted);text-align:right;">${step.transitionDescription || ''}</span>
          <span class="card-badge ${step.isAccepted ? 'emerald' : ''}" style="font-size:9px;">${step.isAccepted ? 'ACC' : '—'}</span>
        </div>`).join('');

      // Highlight last step in DFA
      if (this.visualizer && this.trace.length > 0) {
        const last = this.trace[this.trace.length - 1];
        this.visualizer.highlightState(last.nextState !== undefined ? last.nextState : last.currentState);
      }
    }

    Dashboard.refreshStats();
  }
};
