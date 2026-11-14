/**
 * DNA Sequence Comparator & Mutation Detection Controller
 */

const ComparatorModule = {
  init() {
    const compareBtn = document.getElementById('comparator-run-btn');
    if (compareBtn) {
      compareBtn.addEventListener('click', () => this.runComparison());
    }

    const swapBtn = document.getElementById('comparator-swap-btn');
    if (swapBtn) {
      swapBtn.addEventListener('click', () => {
        const s1 = document.getElementById('comp-seq1-input');
        const s2 = document.getElementById('comp-seq2-input');
        if (s1 && s2) {
          const temp = s1.value;
          s1.value = s2.value;
          s2.value = temp;
        }
      });
    }
  },

  async runComparison() {
    const s1Input = document.getElementById('comp-seq1-input');
    const s2Input = document.getElementById('comp-seq2-input');
    const resultsContainer = document.getElementById('comparator-results-container');

    const seq1 = s1Input ? s1Input.value.trim() : '';
    const seq2 = s2Input ? s2Input.value.trim() : '';

    if (!seq1 || !seq2) {
      App.showToast('Please provide both Sequence 1 and Sequence 2 for comparison.');
      return;
    }

    const res = await API.compareSequences(seq1, seq2);

    if (!res.success) {
      App.showToast(res.errorMessage || 'Comparison failed');
      return;
    }

    if (resultsContainer) resultsContainer.style.display = 'block';

    // Summary Metrics
    document.getElementById('comp-res-similarity').textContent = `${res.similarityPercentage.toFixed(1)}%`;
    document.getElementById('comp-res-diff-status').textContent = res.differenceFound ? 'Differences Detected' : 'Identical Sequences';
    document.getElementById('comp-res-diff-status').style.color = res.differenceFound ? '#D9383A' : '#1E7E34';

    document.getElementById('comp-res-matches').textContent = res.totalMatches;
    document.getElementById('comp-res-subs').textContent = res.totalSubstitutions;
    document.getElementById('comp-res-ins').textContent = res.totalInsertions;
    document.getElementById('comp-res-dels').textContent = res.totalDeletions;

    // Visual Alignment Banner
    const banner = document.getElementById('comp-alignment-banner');
    if (banner) {
      banner.innerHTML = `Seq1 (Ref):   ${res.alignedSeq1}\nAlignment:    ${res.matchSymbols}\nSeq2 (Mut):   ${res.alignedSeq2}`;
    }

    // Differences Table
    const diffBody = document.getElementById('comp-diff-body');
    if (diffBody) {
      if (!res.differences || res.differences.length === 0) {
        diffBody.innerHTML = `<tr><td colspan="6" style="text-align:center; color:#1E7E34; font-weight:600;">No mutations or sequence differences found. Sequences are 100% identical.</td></tr>`;
      } else {
        diffBody.innerHTML = res.differences.map(d => {
          let badgeClass = 'diff-sub';
          if (d.type === 'Insertion') badgeClass = 'diff-ins';
          if (d.type === 'Deletion') badgeClass = 'diff-del';

          return `
            <tr>
              <td><strong>Col #${d.alignmentPosition}</strong></td>
              <td>${d.seq1Pos}</td>
              <td>${d.seq2Pos}</td>
              <td><span class="mono-text" style="font-weight:700;">'${d.char1}'</span></td>
              <td><span class="mono-text" style="font-weight:700;">'${d.char2}'</span></td>
              <td><span class="${badgeClass}">${d.type}</span></td>
              <td style="font-size:12px; color:var(--color-text-muted);">${d.explanation}</td>
            </tr>
          `;
        }).join('');
      }
    }

    Dashboard.refreshStats();
  }
};
