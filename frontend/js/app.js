/**
 * GENOME·AUTOMATA — Main Application Orchestrator
 * Manages navigation, presets, backend health, and module init.
 */

const App = {
  activeTab: 'tab-dashboard',

  init() {
    this.setupNavigation();
    this.setupPresets();

    // Initialize all modules
    Dashboard.init();
    Validator.init();
    PatternModule.init();
    AutomataModule.init();
    SimulatorModule.init();
    ComparatorModule.init();

    // Start DNA helix animation
    if (typeof DNAHelix !== 'undefined') {
      window._dnaHelix = new DNAHelix('dna-helix-canvas');
      window._dnaHelix.start();
    }

    // Heartbeat check
    this.checkBackendHealth();
    setInterval(() => this.checkBackendHealth(), 6000);
  },

  setupNavigation() {
    document.querySelectorAll('.nav-item').forEach(item => {
      item.addEventListener('click', (e) => {
        e.preventDefault();
        const tabId = item.getAttribute('data-tab');
        if (tabId) {
          this.switchTab(tabId);
        }
      });
    });
  },

  switchTab(tabId) {
    this.activeTab = tabId;

    // Update sidebar nav items
    document.querySelectorAll('.nav-item').forEach(n => n.classList.remove('active'));
    const activeNav = document.querySelector(`.nav-item[data-tab="${tabId}"]`);
    if (activeNav) activeNav.classList.add('active');

    // Update tab panels
    document.querySelectorAll('.tab-panel').forEach(p => p.classList.remove('active'));
    const activePanel = document.getElementById(tabId);
    if (activePanel) activePanel.classList.add('active');

    // Update topbar title
    const titleEl = document.getElementById('topbar-title');
    const descEl  = document.getElementById('topbar-desc');
    if (activeNav && titleEl && descEl) {
      titleEl.textContent = activeNav.getAttribute('data-title') || 'GENOME·AUTOMATA';
      descEl.textContent  = activeNav.getAttribute('data-desc')  || 'DNA Sequence Pattern Analyzer & Mutation Detector';
    }

    if (tabId === 'tab-dashboard') {
      Dashboard.refreshStats();
    }
  },

  setupPresets() {
    const selector = document.getElementById('preset-select');
    if (!selector) return;

    selector.addEventListener('change', (e) => {
      const val = e.target.value;
      if (!val) return;

      switch (val) {
        case 'insulin':
          document.getElementById('pattern-seq-input').value    = 'ATGCGATGATG';
          document.getElementById('pattern-target-input').value = 'ATG';
          document.getElementById('sim-seq-input').value        = 'ATGCGATG';
          document.getElementById('sim-regex-source-input').value = 'ATG';
          this.switchTab('tab-pattern');
          PatternModule.runPatternDetection();
          this.showToast('Preset: Insulin Start Codon (ATG)');
          break;

        case 'overlapping':
          document.getElementById('pattern-seq-input').value    = 'ATATAT';
          document.getElementById('pattern-target-input').value = 'ATAT';
          this.switchTab('tab-pattern');
          PatternModule.runPatternDetection();
          this.showToast('Preset: Overlapping Pattern ATAT in ATATAT');
          break;

        case 'tata':
          document.getElementById('regex-input').value           = 'TATAAA';
          document.getElementById('sim-seq-input').value         = 'TATAAA';
          document.getElementById('sim-regex-source-input').value= 'TATAAA';
          this.switchTab('tab-automata');
          AutomataModule.runRegexCompilation();
          this.showToast('Preset: TATA Box Core Promoter');
          break;

        case 'ecori':
          document.getElementById('pattern-seq-input').value    = 'CCGAATTCGG';
          document.getElementById('pattern-target-input').value = 'GAATTC';
          this.switchTab('tab-pattern');
          PatternModule.runPatternDetection();
          this.showToast('Preset: EcoRI Restriction Site (GAATTC)');
          break;

        case 'sickle':
          document.getElementById('comp-seq1-input').value = 'CCTGAGGAG';
          document.getElementById('comp-seq2-input').value = 'CCTGTGGAG';
          this.switchTab('tab-comparator');
          ComparatorModule.runComparison();
          this.showToast('Preset: Sickle-Cell SNV (A→T)');
          break;
      }

      selector.value = '';
    });
  },

  async checkBackendHealth() {
    const dot  = document.getElementById('engine-status-dot');
    const text = document.getElementById('engine-status-text');

    const res = await API.checkHealth();
    if (res && res.status === 'ok') {
      if (dot)  dot.className  = 'status-dot';
      if (text) text.textContent = 'C++17 Engine Connected';
    } else {
      if (dot)  dot.className  = 'status-dot offline';
      if (text) text.textContent = 'Backend Offline';
    }
  },

  showToast(message) {
    const container = document.getElementById('toast-container');
    if (!container) return;
    const toast = document.createElement('div');
    toast.className = 'toast';
    toast.textContent = message;
    container.appendChild(toast);
    setTimeout(() => toast.remove(), 3500);
  }
};

window.addEventListener('DOMContentLoaded', () => {
  App.init();
});
