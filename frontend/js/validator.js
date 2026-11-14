/**
 * DNA Sequence Validator Controller
 */

const Validator = {
  init() {
    const input = document.getElementById('validator-seq-input');
    const validateBtn = document.getElementById('validator-btn');
    const clearBtn = document.getElementById('validator-clear-btn');

    if (validateBtn) {
      validateBtn.addEventListener('click', () => this.runValidation());
    }

    if (clearBtn) {
      clearBtn.addEventListener('click', () => {
        if (input) input.value = '';
        this.clearResults();
      });
    }

    // Quick insertion buttons
    document.querySelectorAll('.nuc-val-insert').forEach(btn => {
      btn.addEventListener('click', (e) => {
        const char = e.target.getAttribute('data-nuc');
        if (input) {
          input.value += char;
          input.focus();
        }
      });
    });

    if (input) {
      input.addEventListener('input', () => {
        document.getElementById('validator-live-len').textContent = input.value.replace(/\s+/g, '').length;
      });
    }
  },

  clearResults() {
    const resBox = document.getElementById('validator-result-box');
    if (resBox) resBox.style.display = 'none';
    document.getElementById('validator-live-len').textContent = '0';
  },

  async runValidation() {
    const input = document.getElementById('validator-seq-input');
    const seq = input ? input.value : '';
    const resBox = document.getElementById('validator-result-box');
    if (!resBox) return;

    const res = await API.validateDNA(seq);
    resBox.style.display = 'block';

    if (res.isValid) {
      resBox.className = 'alert alert-success';
      resBox.innerHTML = `
        <div style="width:100%;">
          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
            <strong style="font-size:14px;">Valid DNA Sequence</strong>
            <span class="card-badge" style="background:#EAF7ED; color:#1E7E34; font-weight:700;">Σ = {A, T, G, C}</span>
          </div>
          <p style="margin-bottom:6px;"><strong>Sanitized Sequence:</strong> <span class="mono-text" style="font-weight:700; color:#101713;">${res.sanitizedSequence}</span></p>
          <p style="font-size:12.5px;"><strong>Sequence Length:</strong> ${res.length} nucleotides</p>
        </div>
      `;
      // Update sanitized value in input
      if (input) input.value = res.sanitizedSequence;
    } else {
      resBox.className = 'alert alert-error';
      resBox.innerHTML = `
        <div>
          <strong style="font-size:14px; display:block; margin-bottom:4px;">Sequence Validation Failed</strong>
          <p>${res.errorMessage}</p>
          ${res.errorPosition > 0 ? `<p style="margin-top:6px; font-size:12px;"><strong>Error Position:</strong> Character ${res.errorPosition} (1-based index) | <strong>Symbol:</strong> '${res.invalidChar}'</p>` : ''}
        </div>
      `;
    }

    Dashboard.refreshStats();
  }
};
