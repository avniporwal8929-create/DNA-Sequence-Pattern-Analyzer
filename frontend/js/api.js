/**
 * API Client for C++ Backend REST Endpoints
 * Communicates strictly via JSON with dna_server.exe
 */

const getApiBaseUrl = () => {
  const custom = localStorage.getItem('GENOME_API_URL');
  if (custom) return custom.replace(/\/+$/, '');

  if (window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1') {
    return window.location.port === '8080' ? window.location.origin : 'http://localhost:8080';
  }
  return window.location.origin;
};

let API_BASE_URL = getApiBaseUrl();

const API = {
  setBaseUrl(url) {
    if (url) {
      localStorage.setItem('GENOME_API_URL', url.replace(/\/+$/, ''));
    } else {
      localStorage.removeItem('GENOME_API_URL');
    }
    API_BASE_URL = getApiBaseUrl();
  },

  getBaseUrl() {
    return API_BASE_URL;
  },
  // Generic helper for JSON POST requests
  async post(endpoint, data) {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(data)
      });
      return await response.json();
    } catch (err) {
      console.error(`API POST error on ${endpoint}:`, err);
      return { success: false, errorMessage: `Backend connection error: ${err.message}` };
    }
  },

  // Generic helper for GET requests
  async get(endpoint) {
    try {
      const response = await fetch(`${API_BASE_URL}${endpoint}`, {
        method: 'GET',
        headers: {
          'Content-Type': 'application/json'
        }
      });
      return await response.json();
    } catch (err) {
      console.error(`API GET error on ${endpoint}:`, err);
      return { status: 'error', errorMessage: err.message };
    }
  },

  // Health check
  async checkHealth() {
    return await this.get('/api/health');
  },

  // Real stats
  async getStats() {
    return await this.get('/api/stats');
  },

  // DNA Sequence Validation
  async validateDNA(sequence) {
    return await this.post('/api/dna/validate', { sequence });
  },

  // Automata-Based Single Pattern Detection
  async detectPattern(sequence, pattern) {
    return await this.post('/api/pattern/detect', { sequence, pattern });
  },

  // Automata-Based Multi-Pattern Detection
  async detectMultiplePatterns(sequence, patterns) {
    return await this.post('/api/pattern/detect-multi', { sequence, patterns });
  },

  // Regex Compilation
  async compileRegex(regex) {
    return await this.post('/api/regex/compile', { regex });
  },

  // Test String Acceptance by Automaton
  async testCandidateString(regex, candidate) {
    return await this.post('/api/automata/test-string', { regex, candidate });
  },

  // Generate NFA
  async generateNFA(input, type = 'regex') {
    return await this.post('/api/automata/nfa', { input, type });
  },

  // NFA to DFA Subset Conversion
  async convertNFAtoDFA(input, type = 'regex') {
    return await this.post('/api/automata/convert', { input, type });
  },

  // DFA Simulation
  async simulateDFA(input, sequence, type = 'regex') {
    return await this.post('/api/automata/simulate', { input, sequence, type });
  },

  // Sequence Comparator / Mutation Detection
  async compareSequences(seq1, seq2) {
    return await this.post('/api/mutation/compare', { seq1, seq2 });
  }
};
