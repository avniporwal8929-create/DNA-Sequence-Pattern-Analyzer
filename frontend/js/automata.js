/**
 * Regular Expression Compiler & NFA-to-DFA Subset Construction Controller
 */

const AutomataModule = {
  nfaVisualizer: null,
  dfaVisualizer: null,
  conversionSteps: [],
  currentStepIndex: 0,

  init() {
    this.nfaVisualizer = new AutomataVisualizer('regex-nfa-viz');
    this.dfaVisualizer = new AutomataVisualizer('regex-dfa-viz');

    const compileBtn = document.getElementById('regex-compile-btn');
    if (compileBtn) {
      compileBtn.addEventListener('click', () => this.runRegexCompilation());
    }

    // Step-by-step subset inspection controls
    const prevBtn = document.getElementById('step-prev-btn');
    const nextBtn = document.getElementById('step-next-btn');

    if (prevBtn) {
      prevBtn.addEventListener('click', () => {
        if (this.currentStepIndex > 0) {
          this.currentStepIndex--;
          this.displayStep(this.currentStepIndex);
        }
      });
    }

    if (nextBtn) {
      nextBtn.addEventListener('click', () => {
        if (this.currentStepIndex < this.conversionSteps.length - 1) {
          this.currentStepIndex++;
          this.displayStep(this.currentStepIndex);
        }
      });
    }
  },

  async runRegexCompilation() {
    const input = document.getElementById('regex-input');
    const regexStr = input ? input.value.trim() : '';
    const errBox = document.getElementById('regex-error-box');
    const resultsContainer = document.getElementById('regex-results-container');

    if (!regexStr) {
      App.showToast('Please enter a regular expression.');
      return;
    }

    if (errBox) errBox.style.display = 'none';

    const res = await API.compileRegex(regexStr);

    if (!res.success) {
      if (errBox) {
        errBox.style.display = 'block';
        errBox.className = 'alert alert-error';
        errBox.innerHTML = `<strong>Regex Syntax Error:</strong> ${res.errorMessage}`;
      }
      return;
    }

    if (resultsContainer) resultsContainer.style.display = 'block';

    // Display parsing metadata
    document.getElementById('regex-meta-original').textContent = res.parsed.originalRegex;
    document.getElementById('regex-meta-explicit').textContent = res.parsed.explicitConcatRegex;
    document.getElementById('regex-meta-postfix').textContent = res.parsed.postfixRegex;

    // Render NFA
    if (res.nfa && this.nfaVisualizer) {
      this.nfaVisualizer.render(res.nfa, `Thompson NFA: ${regexStr}`);
      this.renderNfaTransitionTable(res.nfa.transitionTable);
    }

    // Render DFA
    if (res.dfa && this.dfaVisualizer) {
      this.dfaVisualizer.render(res.dfa, `Equivalent DFA: ${regexStr}`);
    }

    // Render Conversion Table
    if (res.conversionTable) {
      this.renderConversionTable(res.conversionTable);
    }

    // Setup Step-by-Step Inspector
    if (res.steps && res.steps.length > 0) {
      this.conversionSteps = res.steps;
      this.currentStepIndex = 0;
      this.displayStep(0);
    }

    // Setup Simulator with this DFA
    SimulatorModule.loadDFA(res.dfa, regexStr);

    Dashboard.refreshStats();
  },

  renderNfaTransitionTable(tableRows) {
    const body = document.getElementById('nfa-table-body');
    if (!body || !tableRows) return;

    body.innerHTML = tableRows.map(row => {
      const trans = row.transitions || {};
      const formatCell = list => (list && list.length > 0) ? `{${list.join(', ')}}` : '∅';
      return `
        <tr>
          <td>
            <strong>${row.stateLabel}</strong>
            ${row.isStart ? '<span class="badge-start"> (Start)</span>' : ''}
            ${row.isAccept ? '<span class="badge-accept"> (Accept)</span>' : ''}
          </td>
          <td>${formatCell(trans['A'])}</td>
          <td>${formatCell(trans['T'])}</td>
          <td>${formatCell(trans['G'])}</td>
          <td>${formatCell(trans['C'])}</td>
          <td style="color:#1D6FB8;">${formatCell(trans['ε'])}</td>
        </tr>
      `;
    }).join('');
  },

  renderConversionTable(tableRows) {
    const body = document.getElementById('conversion-table-body');
    if (!body || !tableRows) return;

    body.innerHTML = tableRows.map(row => {
      const trans = row.transitions || {};
      return `
        <tr>
          <td>
            <strong>${row.dfaState}</strong>
            ${row.isStart ? '<span class="badge-start"> (Initial)</span>' : ''}
            ${row.isAccept ? '<span class="badge-accept"> (Accept)</span>' : ''}
          </td>
          <td class="mono-text" style="color:#556B5F;">${row.nfaSubset}</td>
          <td>${trans['A'] || '∅'}</td>
          <td>${trans['T'] || '∅'}</td>
          <td>${trans['G'] || '∅'}</td>
          <td>${trans['C'] || '∅'}</td>
        </tr>
      `;
    }).join('');
  },

  displayStep(index) {
    if (!this.conversionSteps || index >= this.conversionSteps.length) return;
    const step = this.conversionSteps[index];

    document.getElementById('step-indicator').textContent = `Step ${index + 1} of ${this.conversionSteps.length}`;
    document.getElementById('step-prev-btn').disabled = (index === 0);
    document.getElementById('step-next-btn').disabled = (index === this.conversionSteps.length - 1);

    const card = document.getElementById('step-details-card');
    if (!card) return;

    let transHtml = '';
    (step.transitions || []).forEach(t => {
      transHtml += `
        <div style="background:#FAFAF8; border:1px solid var(--color-border); border-radius:4px; padding:10px; margin-bottom:8px;">
          <div style="display:flex; justify-content:space-between; margin-bottom:4px;">
            <strong>Symbol '${t.symbol}':</strong>
            <span class="card-badge" style="${t.isNewState ? 'background:#EAF7ED; color:#1E7E34; font-weight:700;' : ''}">
              ${t.isNewState ? 'New State Created' : 'Existing State'}
            </span>
          </div>
          <p style="font-size:12.5px; margin-bottom:2px;">• Move(${step.nfaSubsetString}, '${t.symbol}') = <code>${t.moveSetString}</code></p>
          <p style="font-size:12.5px; margin-bottom:4px;">• ε-closure(${t.moveSetString}) = <code>${t.closureSetString}</code> → Target DFA State: <strong>${t.targetStateLabel}</strong></p>
          <p style="font-size:11.5px; color:var(--color-text-muted);">${t.explanation}</p>
        </div>
      `;
    });

    card.innerHTML = `
      <div style="margin-bottom:12px;">
        <h4 style="font-size:15px; margin-bottom:4px;">
          Processing DFA State: <strong>${step.dfaStateLabel}</strong>
          ${step.isStart ? '<span class="badge-start">(Start)</span>' : ''}
          ${step.isAccept ? '<span class="badge-accept">(Accepting State)</span>' : ''}
        </h4>
        <p style="font-size:13px; color:var(--color-text-muted);">Corresponding NFA State Subset: <code class="mono-text" style="font-weight:700;">${step.nfaSubsetString}</code></p>
      </div>
      <h5 style="font-size:13px; font-weight:700; margin-bottom:8px; text-transform:uppercase; letter-spacing:0.5px; color:#556B5F;">Transitions Computed for DNA Alphabet {A, T, G, C}:</h5>
      ${transHtml}
    `;

    // Highlight this state in the DFA visualizer
    if (this.dfaVisualizer) {
      this.dfaVisualizer.highlightState(step.dfaStateId);
    }
  }
};
