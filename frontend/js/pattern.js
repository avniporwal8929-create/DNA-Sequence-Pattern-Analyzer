/**
 * Automata-Based Pattern Detection Controller
 * Supports single-pattern and multi-pattern searching via C++
 */

const PatternModule = {
  visualizer: null,

  init() {
    this.visualizer = new AutomataVisualizer('pattern-automata-viz');

    const searchBtn = document.getElementById('pattern-search-btn');
    if (searchBtn) {
      searchBtn.addEventListener('click', () => this.runPatternDetection());
    }

    const multiBtn = document.getElementById('pattern-multi-search-btn');
    if (multiBtn) {
      multiBtn.addEventListener('click', () => this.runMultiPatternDetection());
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

    // Check if user entered comma-separated multiple patterns
    if (pattern.includes(',')) {
      return this.runMultiPatternDetection();
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
  },

  async runMultiPatternDetection() {
    const seqInput = document.getElementById('pattern-seq-input');
    const patInput = document.getElementById('pattern-target-input');
    const resultsCard = document.getElementById('pattern-results-card');

    const sequence = seqInput ? seqInput.value.trim() : '';
    const rawPattern = patInput ? patInput.value.trim() : '';

    if (!sequence || !rawPattern) {
      App.showToast('Please provide both a DNA sequence and target patterns.');
      return;
    }

    const patterns = rawPattern.split(/[,;\s]+/).filter(p => p.length > 0);
    if (patterns.length === 0) return;

    const res = await API.detectMultiplePatterns(sequence, patterns);
    if (!res.success) {
      App.showToast(res.errorMessage || 'Multi-pattern detection failed');
      return;
    }

    if (resultsCard) resultsCard.style.display = 'block';

    document.getElementById('pat-res-found').textContent = res.totalOccurrencesCombined > 0 ? 'Yes' : 'No';
    document.getElementById('pat-res-found').style.color = res.totalOccurrencesCombined > 0 ? '#1E7E34' : '#D9383A';
    document.getElementById('pat-res-count').textContent = `${res.totalOccurrencesCombined} (across ${patterns.length} patterns)`;
    
    // Collect starting positions
    const allPositions = [];
    const allOccurrences = [];
    res.patternResults.forEach(pr => {
      pr.startingPositions.forEach(p => allPositions.push(`${p} (${pr.pattern})`));
      pr.occurrences.forEach(o => allOccurrences.push(o));
    });

    document.getElementById('pat-res-positions').textContent = allPositions.length > 0 ? allPositions.join(', ') : 'None';

    // Highlighted visual sequence rendering
    const hlContainer = document.getElementById('pattern-highlighted-view');
    if (hlContainer && res.dnaSequence) {
      let html = '';
      const mask = res.combinedMatchMask || [];
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
      if (allOccurrences.length === 0) {
        tableBody.innerHTML = `<tr><td colspan="4" style="text-align:center; color:var(--color-text-muted);">No patterns found in sequence.</td></tr>`;
      } else {
        tableBody.innerHTML = allOccurrences.map((o, idx) => `
          <tr>
            <td>#${idx + 1}</td>
            <td><strong>Position ${o.startIndex}</strong></td>
            <td>Position ${o.endIndex}</td>
            <td><span class="match-hl mono-text">${o.matchedText}</span> <span style="font-size:11px; color:#556B5F;">[${o.patternTag || ''}]</span></td>
          </tr>
        `).join('');
      }
    }

    // Render first pattern's DFA in visualizer
    if (res.patternResults.length > 0 && res.patternResults[0].matcherDFA && this.visualizer) {
      this.visualizer.render(res.patternResults[0].matcherDFA, `DFA for Pattern: "${res.patternResults[0].pattern}"`);
    }

    Dashboard.refreshStats();
  }
};
