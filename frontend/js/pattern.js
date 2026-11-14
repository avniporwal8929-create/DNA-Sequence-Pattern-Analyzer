/**
 * Automata-Based Pattern Detection Controller
 */

const PatternModule = {
  visualizer: null,

  init() {
    this.visualizer = new AutomataVisualizer('pattern-automata-viz');

    const searchBtn = document.getElementById('pattern-search-btn');
    if (searchBtn) {
      searchBtn.addEventListener('click', () => this.runPatternDetection());
    }

    // Quick nucleotide insertion for pattern
    document.querySelectorAll('.nuc-pat-insert').forEach(btn => {
      btn.addEventListener('click', (e) => {
        const char = e.target.getAttribute('data-nuc');
        const input = document.getElementById('pattern-target-input');
        if (input) {
          input.value += char;
          input.focus();
        }
      });
    });
  },

  async runPatternDetection() {
    const seqInput = document.getElementById('pattern-seq-input');
    const patInput = document.getElementById('pattern-target-input');
    const resultsCard = document.getElementById('pattern-results-card');

    const sequence = seqInput ? seqInput.value.trim() : '';
    const pattern = patInput ? patInput.value.trim() : '';

    if (!sequence || !pattern) {
      App.showToast('Please provide both a DNA sequence and a target pattern.');
      return;
    }

    const res = await API.detectPattern(sequence, pattern);
    if (!res.success) {
      App.showToast(res.errorMessage || 'Pattern detection failed');
      return;
    }

    if (resultsCard) resultsCard.style.display = 'block';

    // Summary metrics
    document.getElementById('pat-res-found').textContent = res.patternFound ? 'Yes' : 'No';
    document.getElementById('pat-res-found').style.color = res.patternFound ? '#1E7E34' : '#D9383A';
    document.getElementById('pat-res-count').textContent = res.totalOccurrences;
    document.getElementById('pat-res-positions').textContent = res.startingPositions.length > 0 
      ? res.startingPositions.join(', ') 
      : 'None';

    // Highlighted visual sequence rendering
    const hlContainer = document.getElementById('pattern-highlighted-view');
    if (hlContainer && res.dnaSequence) {
      let html = '';
      const mask = res.matchMask || [];
      for (let i = 0; i < res.dnaSequence.length; i++) {
        const char = res.dnaSequence[i];
        if (mask[i]) {
          html += `<span class="match-hl">${char}</span>`;
        } else {
          html += char;
        }
      }
      hlContainer.innerHTML = html;
    }

    // Occurrences Table
    const tableBody = document.getElementById('pattern-occurrences-body');
    if (tableBody) {
      if (res.occurrences.length === 0) {
        tableBody.innerHTML = `<tr><td colspan="4" style="text-align:center; color:var(--color-text-muted);">Pattern not found in sequence.</td></tr>`;
      } else {
        tableBody.innerHTML = res.occurrences.map(o => `
          <tr>
            <td>#${o.matchNumber}</td>
            <td><strong>Position ${o.startIndex}</strong></td>
            <td>Position ${o.endIndex}</td>
            <td><span class="match-hl mono-text">${o.matchedText}</span></td>
          </tr>
        `).join('');
      }
    }

    // Render Pattern Matcher DFA
    if (res.matcherDFA && this.visualizer) {
      this.visualizer.render(res.matcherDFA, `DFA for Pattern: "${res.pattern}"`);
    }

    Dashboard.refreshStats();
  }
};
