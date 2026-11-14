/**
 * Interactive DFA Simulator Controller
 * Executes step-by-step state machine transitions on DNA strings
 */

const SimulatorModule = {
  visualizer: null,
  currentDFA: null,
  currentPatternName: '',
  trace: [],
  currentStepIndex: 0,
  autoRunTimer: null,
  isRunning: false,

  init() {
    this.visualizer = new AutomataVisualizer('sim-automata-viz');

    document.getElementById('sim-start-btn')?.addEventListener('click', () => this.startSimulation());
    document.getElementById('sim-next-btn')?.addEventListener('click', () => this.stepNext());
    document.getElementById('sim-prev-btn')?.addEventListener('click', () => this.stepPrev());
    document.getElementById('sim-reset-btn')?.addEventListener('click', () => this.resetSimulation());
    document.getElementById('sim-autorun-btn')?.addEventListener('click', () => this.toggleAutoRun());
  },

  loadDFA(dfa, patternName = "") {
    this.currentDFA = dfa;
    this.currentPatternName = patternName;
    if (this.visualizer && dfa) {
      this.visualizer.render(dfa, `DFA Simulator: ${patternName || 'Active Automaton'}`);
    }
    const info = document.getElementById('sim-automaton-info');
    if (info) {
      info.textContent = `Loaded DFA for: ${patternName || 'Current Automaton'} (${dfa.states.length} states)`;
    }
  },

  async startSimulation() {
    const seqInput = document.getElementById('sim-seq-input');
    const seq = seqInput ? seqInput.value.trim() : '';

    if (!seq) {
      App.showToast('Please enter a DNA sequence to simulate.');
      return;
    }

    const regInput = document.getElementById('sim-regex-source-input');
    const regexPattern = (regInput && regInput.value.trim()) ? regInput.value.trim() : 'ATG';

    const res = await API.simulateDFA(regexPattern, seq, 'regex');

    if (!res.success) {
      App.showToast(res.errorMessage || 'Simulation failed');
      return;
    }

    this.currentDFA = res.dfa;
    this.visualizer.render(res.dfa, `DFA: ${regexPattern}`);
    this.trace = res.trace || [];
    this.currentStepIndex = 0;

    document.getElementById('sim-controls-panel').style.display = 'flex';
    document.getElementById('sim-trace-table-container').style.display = 'block';

    this.renderTraceTable();
    this.goToStep(0);

    Dashboard.refreshStats();
  },

  renderTraceTable() {
    const body = document.getElementById('sim-trace-body');
    if (!body) return;

    body.innerHTML = this.trace.map((step, idx) => `
      <tr id="sim-trace-row-${idx}" style="cursor:pointer;" onclick="SimulatorModule.goToStep(${idx})">
        <td><strong>Step ${step.stepNumber}</strong></td>
        <td><span class="mono-text">${step.currentStateLabel} ${step.currentSubset}</span></td>
        <td><strong class="mono-text" style="color:#101713;">${step.inputSymbol}</strong></td>
        <td><span class="mono-text">${step.nextStateLabel}</span></td>
        <td style="font-size:12px; color:var(--color-text-muted);">${step.transitionDescription}</td>
        <td>
          <span class="card-badge" style="${step.isAccepted ? 'background:#EAF7ED; color:#1E7E34; font-weight:700;' : ''}">
            ${step.isAccepted ? 'ACCEPT' : 'Non-Accept'}
          </span>
        </td>
      </tr>
    `).join('');
  },

  goToStep(index) {
    if (index < 0 || index >= this.trace.length) return;
    this.currentStepIndex = index;
    const step = this.trace[index];

    // Highlight row in table
    document.querySelectorAll('#sim-trace-body tr').forEach(r => r.style.backgroundColor = '');
    const activeRow = document.getElementById(`sim-trace-row-${index}`);
    if (activeRow) {
      activeRow.style.backgroundColor = '#F0F9EA';
    }

    // Update status indicators
    document.getElementById('sim-step-indicator').textContent = `Step ${index} of ${this.trace.length - 1}`;
    document.getElementById('sim-curr-state-val').textContent = `${step.currentStateLabel} ${step.currentSubset}`;
    document.getElementById('sim-curr-sym-val').textContent = step.inputSymbol;
    document.getElementById('sim-next-state-val').textContent = step.nextStateLabel;
    
    const accBadge = document.getElementById('sim-status-badge');
    if (accBadge) {
      accBadge.textContent = step.isAccepted ? 'ACCEPTED' : 'PENDING / REJECT';
      accBadge.style.backgroundColor = step.isAccepted ? 'var(--color-accent-lime)' : 'var(--color-workspace)';
      accBadge.style.color = '#101713';
    }

    // Highlight active state and transition in SVG
    if (this.visualizer) {
      this.visualizer.highlightState(step.currentState);
      if (step.nextState !== -1 && step.nextState !== step.currentState) {
        this.visualizer.highlightTransition(step.currentState, step.nextState);
      }
    }
  },

  stepNext() {
    if (this.currentStepIndex < this.trace.length - 1) {
      this.goToStep(this.currentStepIndex + 1);
    } else {
      if (this.isRunning) this.stopAutoRun();
    }
  },

  stepPrev() {
    if (this.currentStepIndex > 0) {
      this.goToStep(this.currentStepIndex - 1);
    }
  },

  resetSimulation() {
    this.stopAutoRun();
    this.goToStep(0);
  },

  toggleAutoRun() {
    if (this.isRunning) {
      this.stopAutoRun();
    } else {
      this.startAutoRun();
    }
  },

  startAutoRun() {
    this.isRunning = true;
    const btn = document.getElementById('sim-autorun-btn');
    if (btn) {
      btn.textContent = 'Pause';
      btn.className = 'btn btn-primary btn-sm';
    }

    this.autoRunTimer = setInterval(() => {
      if (this.currentStepIndex < this.trace.length - 1) {
        this.stepNext();
      } else {
        this.stopAutoRun();
      }
    }, 800);
  },

  stopAutoRun() {
    this.isRunning = false;
    clearInterval(this.autoRunTimer);
    const btn = document.getElementById('sim-autorun-btn');
    if (btn) {
      btn.textContent = 'Auto Run';
      btn.className = 'btn btn-outline btn-sm';
    }
  }
};
