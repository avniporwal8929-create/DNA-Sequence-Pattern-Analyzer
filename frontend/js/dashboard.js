/**
 * Dashboard & Session Analytics Controller
 * Displays real-time application metrics from the C++ backend
 */

const Dashboard = {
  async init() {
    this.refreshStats();
  },

  async refreshStats() {
    const stats = await API.getStats();
    if (stats && stats.totalSequencesAnalyzed !== undefined) {
      const elSeqs = document.getElementById('stat-sequences');
      const elValid = document.getElementById('stat-valid-seqs');
      const elInvalid = document.getElementById('stat-invalid-seqs');
      const elPatterns = document.getElementById('stat-patterns');
      const elMutations = document.getElementById('stat-mutations');
      const elAutomata = document.getElementById('stat-automata');

      if (elSeqs) elSeqs.textContent = stats.totalSequencesAnalyzed;
      if (elValid) elValid.textContent = stats.validSequencesCount || 0;
      if (elInvalid) elInvalid.textContent = stats.invalidSequencesCount || 0;
      if (elPatterns) elPatterns.textContent = stats.totalPatternsDetected;
      if (elMutations) elMutations.textContent = stats.totalMutationsIdentified || 0;
      if (elAutomata) elAutomata.textContent = stats.totalAutomataGenerated || 0;

      const historyBody = document.getElementById('dashboard-history-body');
      if (historyBody) {
        if (!stats.history || stats.history.length === 0) {
          historyBody.innerHTML = `<tr><td colspan="3" style="text-align:center; color:var(--color-text-muted);">No activity recorded yet in this session. Start by analyzing a sequence!</td></tr>`;
        } else {
          historyBody.innerHTML = stats.history.map(item => `
            <tr>
              <td><span class="card-badge">${item.time}</span></td>
              <td><strong>${item.action}</strong></td>
              <td style="color:var(--color-text-muted); font-size:12px;">${item.details}</td>
            </tr>
          `).join('');
        }
      }
    }
  }
};
