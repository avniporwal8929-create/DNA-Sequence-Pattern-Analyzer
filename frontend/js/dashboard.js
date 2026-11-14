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
      document.getElementById('stat-sequences').textContent = stats.totalSequencesAnalyzed;
      document.getElementById('stat-patterns').textContent = stats.totalPatternsDetected;
      document.getElementById('stat-automata').textContent = stats.totalAutomataGenerated;
      document.getElementById('stat-mutations').textContent = stats.totalMutationsCompared;

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
