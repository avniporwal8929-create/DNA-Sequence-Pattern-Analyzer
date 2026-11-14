/**
 * Regular Expression Compiler & NFA→DFA Lab Controller
 * Updated for GENOME·AUTOMATA dark biotech UI
 */

const AutomataModule = {
  nfaVisualizer: null,
  dfaVisualizer: null,
  currentCompiledRegex: '',

  init() {
    // Visualizers mapped to new container IDs
    this.nfaVisualizer = new AutomataVisualizer('nfa-canvas-container');
    this.dfaVisualizer = new AutomataVisualizer('dfa-canvas-container');

    document.getElementById('compile-regex-btn')?.addEventListener('click', () => this.runRegexCompilation());
    document.getElementById('convert-dfa-btn')?.addEventListener('click', () => this.runNFAtoDFA());
    document.getElementById('test-string-btn')?.addEventListener('click', () => this.testCandidateString());
  },

  async runRegexCompilation() {
    const input    = document.getElementById('regex-input');
    const regexStr = input ? input.value.trim() : '';
    if (!regexStr) { App.showToast('Please enter a regular expression.'); return; }

    this.currentCompiledRegex = regexStr;

    const res = await API.compileRegex(regexStr);
    if (!res.success) {
      App.showToast('Regex Error: ' + (res.errorMessage || 'Compilation failed'));
      return;
    }

    // Render NFA
    if (res.nfa && this.nfaVisualizer) {
      this.nfaVisualizer.render(res.nfa, `Thompson NFA: ${regexStr}`);
    }

    // Render DFA
    if (res.dfa && this.dfaVisualizer) {
      this.dfaVisualizer.render(res.dfa, `Subset DFA: ${regexStr}`);
    }

    // Transition table
    this.renderTransitionTable(res.dfa || res.nfa, regexStr);

    // Pass to Simulator
    if (res.dfa) SimulatorModule.loadDFA(res.dfa, regexStr);

    Dashboard.refreshStats();
  },

  async runNFAtoDFA() {
    const input    = document.getElementById('regex-input');
    const regexStr = input ? input.value.trim() : '';
    if (!regexStr) { App.showToast('Enter a regex first, then convert.'); return; }

    this.currentCompiledRegex = regexStr;
    const res = await API.convertNFAtoDFA(regexStr, 'regex');

    if (!res.success) {
      App.showToast('Conversion error: ' + (res.errorMessage || 'NFA→DFA failed'));
      return;
    }

    if (res.dfa && this.dfaVisualizer) {
      this.dfaVisualizer.render(res.dfa, `DFA (Subset): ${regexStr}`);
    }

    this.renderTransitionTable(res.dfa, regexStr);

    if (res.dfa) SimulatorModule.loadDFA(res.dfa, regexStr);

    Dashboard.refreshStats();
  },

  async testCandidateString() {
    const candidate = document.getElementById('regex-test-input')?.value.trim() || '';
    const regexStr  = this.currentCompiledRegex || document.getElementById('regex-input')?.value.trim() || '';
    const resBox    = document.getElementById('regex-test-result');

    if (!regexStr) { App.showToast('Compile a regex first.'); return; }

    const res = await API.testCandidateString(regexStr, candidate);
    if (!resBox) return;

    if (res.success) {
      if (res.isAccepted) {
        resBox.innerHTML = `<div class="verdict-accepted">✓ ACCEPTED — "<span class="text-mono">${candidate || 'ε'}</span>" reaches an accepting state.</div>`;
      } else {
        resBox.innerHTML = `<div class="verdict-rejected">✗ REJECTED — "<span class="text-mono">${candidate || 'ε'}</span>" halts in a non-accepting state.</div>`;
      }
    } else {
      resBox.innerHTML = `<div class="verdict-rejected">Error: ${res.errorMessage}</div>`;
    }

    Dashboard.refreshStats();
  },

  renderTransitionTable(automaton, label) {
    const container = document.getElementById('automata-table-container');
    const badge     = document.getElementById('automata-table-badge');
    if (!container || !automaton) return;

    if (badge) badge.textContent = `${automaton.type || 'Automaton'} · ${(automaton.states || []).length} states`;

    const table = automaton.transitionTable || [];
    if (table.length === 0) {
      container.innerHTML = `<div style="text-align:center;padding:24px;color:var(--text-muted);font-size:12px;">No transition table available.</div>`;
      return;
    }

    const formatCell = list => (list && list.length > 0) ? `{${list.join(', ')}}` : '∅';

    container.innerHTML = `
      <table class="transition-table">
        <thead>
          <tr>
            <th>State</th>
            <th style="color:var(--cyan)">A</th>
            <th style="color:var(--amber)">T</th>
            <th style="color:var(--emerald)">G</th>
            <th style="color:var(--purple)">C</th>
            <th style="color:var(--text-muted)">ε</th>
          </tr>
        </thead>
        <tbody>
          ${table.map(row => {
            const t = row.transitions || {};
            const startCls = row.isStart  ? 'start-state' : '';
            const accCls   = row.isAccept ? 'accept-state' : '';
            return `
              <tr>
                <td class="${startCls || accCls}">
                  ${row.isStart  ? '→ ' : ''}${row.stateLabel}${row.isAccept ? ' *' : ''}
                </td>
                <td>${formatCell(t['A'])}</td>
                <td>${formatCell(t['T'])}</td>
                <td>${formatCell(t['G'])}</td>
                <td>${formatCell(t['C'])}</td>
                <td style="color:var(--text-muted)">${formatCell(t['ε'])}</td>
              </tr>`;
          }).join('')}
        </tbody>
      </table>`;
  }
};
