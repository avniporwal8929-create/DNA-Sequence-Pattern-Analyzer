/**
 * GENOME·AUTOMATA — Animated DNA Double Helix Visualizer
 * Canvas-based, scientifically styled, smooth requestAnimationFrame loop.
 * Nucleotide colors: A=Cyan, T=Amber/Orange, G=Emerald, C=Purple
 */

class DNAHelix {
  constructor(canvasId) {
    this.canvas = document.getElementById(canvasId);
    if (!this.canvas) return;
    this.ctx = this.canvas.getContext('2d');
    this.t = 0;
    this.raf = null;

    // Colors matching design system
    this.colors = {
      A: '#38C8F4', // Cyan
      T: '#F4A338', // Amber
      G: '#35D995', // Emerald
      C: '#9B6DF4', // Purple
      backbone: 'rgba(56, 200, 244, 0.25)',
      backbone2: 'rgba(53, 217, 149, 0.2)',
      rung: 'rgba(100, 150, 180, 0.25)',
      glow: 'rgba(56, 200, 244, 0.08)',
    };

    // DNA sequence to visualize
    this.sequence = 'ATGCGATGATGCTTAATCGATCGGCATCG'.split('');
    this.nucleotideCount = this.sequence.length;

    this.resize();
    window.addEventListener('resize', () => this.resize());
  }

  resize() {
    if (!this.canvas) return;
    const rect = this.canvas.parentElement.getBoundingClientRect();
    this.canvas.width = rect.width;
    this.canvas.height = rect.height;
    this.W = this.canvas.width;
    this.H = this.canvas.height;
  }

  getNucleotideColor(nt) {
    return this.colors[nt] || '#7DC4D8';
  }

  getComplement(nt) {
    const map = { A: 'T', T: 'A', G: 'C', C: 'G' };
    return map[nt] || nt;
  }

  draw() {
    if (!this.canvas || !this.ctx) return;
    const ctx = this.ctx;
    const W = this.W;
    const H = this.H;

    ctx.clearRect(0, 0, W, H);

    // Background gradient
    const bg = ctx.createLinearGradient(0, 0, 0, H);
    bg.addColorStop(0, 'rgba(10, 24, 40, 0)');
    bg.addColorStop(1, 'rgba(10, 24, 40, 0)');
    ctx.fillStyle = bg;
    ctx.fillRect(0, 0, W, H);

    const amplitude = Math.min(W * 0.18, 70);
    const centerX = W / 2;
    const numRungs = this.nucleotideCount;
    const spacing = H / (numRungs + 1);
    const phase = this.t;

    // Collect strand 1 and strand 2 y-mapped points
    const strand1 = [];
    const strand2 = [];

    for (let i = 0; i < numRungs; i++) {
      const y = spacing * (i + 1);
      const angle = (i / numRungs) * Math.PI * 4 + phase;
      const x1 = centerX + Math.sin(angle) * amplitude;
      const x2 = centerX - Math.sin(angle) * amplitude;
      strand1.push({ x: x1, y, angle });
      strand2.push({ x: x2, y, angle });
    }

    // Draw background glow behind helix
    ctx.save();
    ctx.globalAlpha = 0.07;
    const glowGrad = ctx.createRadialGradient(centerX, H / 2, 10, centerX, H / 2, amplitude + 60);
    glowGrad.addColorStop(0, '#38C8F4');
    glowGrad.addColorStop(1, 'transparent');
    ctx.fillStyle = glowGrad;
    ctx.fillRect(0, 0, W, H);
    ctx.restore();

    // Draw rungs (base pairs) BEHIND strands
    for (let i = 0; i < numRungs; i++) {
      const p1 = strand1[i];
      const p2 = strand2[i];
      const nt = this.sequence[i % this.sequence.length];
      const nt2 = this.getComplement(nt);
      const c1 = this.getNucleotideColor(nt);
      const c2 = this.getNucleotideColor(nt2);

      // Depth: fade rungs that are "behind"
      const cosVal = Math.cos(p1.angle);
      const alpha = 0.1 + 0.5 * (1 + cosVal) / 2;

      // Rung line
      const rungGrad = ctx.createLinearGradient(p1.x, p1.y, p2.x, p2.y);
      rungGrad.addColorStop(0, c1 + '55');
      rungGrad.addColorStop(0.4, 'rgba(100,140,170,0.15)');
      rungGrad.addColorStop(0.6, 'rgba(100,140,170,0.15)');
      rungGrad.addColorStop(1, c2 + '55');

      ctx.save();
      ctx.globalAlpha = alpha;
      ctx.strokeStyle = rungGrad;
      ctx.lineWidth = 2.5;
      ctx.beginPath();
      ctx.moveTo(p1.x, p1.y);
      ctx.lineTo(p2.x, p2.y);
      ctx.stroke();
      ctx.restore();
    }

    // Draw strand 1 smooth bezier curve
    ctx.save();
    ctx.globalAlpha = 0.9;
    ctx.lineWidth = 2.8;
    const s1Grad = ctx.createLinearGradient(0, 0, 0, H);
    s1Grad.addColorStop(0, '#38C8F4');
    s1Grad.addColorStop(0.5, '#35D995');
    s1Grad.addColorStop(1, '#38C8F4');
    ctx.strokeStyle = s1Grad;
    ctx.shadowColor = '#38C8F4';
    ctx.shadowBlur = 8;
    ctx.beginPath();
    ctx.moveTo(strand1[0].x, strand1[0].y);
    for (let i = 1; i < strand1.length - 1; i++) {
      const mx = (strand1[i].x + strand1[i + 1].x) / 2;
      const my = (strand1[i].y + strand1[i + 1].y) / 2;
      ctx.quadraticCurveTo(strand1[i].x, strand1[i].y, mx, my);
    }
    ctx.stroke();
    ctx.restore();

    // Draw strand 2 smooth bezier curve
    ctx.save();
    ctx.globalAlpha = 0.9;
    ctx.lineWidth = 2.8;
    const s2Grad = ctx.createLinearGradient(0, 0, 0, H);
    s2Grad.addColorStop(0, '#35D995');
    s2Grad.addColorStop(0.5, '#38C8F4');
    s2Grad.addColorStop(1, '#35D995');
    ctx.strokeStyle = s2Grad;
    ctx.shadowColor = '#35D995';
    ctx.shadowBlur = 8;
    ctx.beginPath();
    ctx.moveTo(strand2[0].x, strand2[0].y);
    for (let i = 1; i < strand2.length - 1; i++) {
      const mx = (strand2[i].x + strand2[i + 1].x) / 2;
      const my = (strand2[i].y + strand2[i + 1].y) / 2;
      ctx.quadraticCurveTo(strand2[i].x, strand2[i].y, mx, my);
    }
    ctx.stroke();
    ctx.restore();

    // Draw nucleotide dots on each node
    for (let i = 0; i < numRungs; i++) {
      const p1 = strand1[i];
      const p2 = strand2[i];
      const nt = this.sequence[i % this.sequence.length];
      const nt2 = this.getComplement(nt);
      const c1 = this.getNucleotideColor(nt);
      const c2 = this.getNucleotideColor(nt2);

      const cosVal = Math.cos(p1.angle);
      const inFront = cosVal > 0;
      const alpha = inFront ? 0.95 : 0.4;
      const radius = inFront ? 5.5 : 3.5;

      // Strand 1 node
      ctx.save();
      ctx.globalAlpha = alpha;
      ctx.beginPath();
      ctx.arc(p1.x, p1.y, radius, 0, Math.PI * 2);
      ctx.fillStyle = c1;
      if (inFront) {
        ctx.shadowColor = c1;
        ctx.shadowBlur = 10;
      }
      ctx.fill();
      ctx.restore();

      // Strand 2 node
      ctx.save();
      ctx.globalAlpha = alpha;
      ctx.beginPath();
      ctx.arc(p2.x, p2.y, radius, 0, Math.PI * 2);
      ctx.fillStyle = c2;
      if (inFront) {
        ctx.shadowColor = c2;
        ctx.shadowBlur = 10;
      }
      ctx.fill();
      ctx.restore();

      // Nucleotide label for visible (front) nodes near center
      if (inFront && radius > 4 && i % 3 === 0) {
        ctx.save();
        ctx.globalAlpha = 0.85;
        ctx.fillStyle = c1;
        ctx.font = '600 9px JetBrains Mono, monospace';
        ctx.textAlign = 'center';
        ctx.textBaseline = 'middle';
        ctx.fillText(nt, p1.x, p1.y);
        ctx.fillStyle = c2;
        ctx.fillText(nt2, p2.x, p2.y);
        ctx.restore();
      }
    }

    // Advance time
    this.t += 0.012;
  }

  start() {
    if (!this.canvas) return;
    const loop = () => {
      this.draw();
      this.raf = requestAnimationFrame(loop);
    };
    this.raf = requestAnimationFrame(loop);
  }

  stop() {
    if (this.raf) cancelAnimationFrame(this.raf);
  }

  updateSequence(seq) {
    if (seq && seq.length > 0) {
      this.sequence = seq.toUpperCase().split('').filter(c => 'ATGC'.includes(c));
      if (this.sequence.length === 0) this.sequence = ['A', 'T', 'G', 'C'];
      this.nucleotideCount = Math.min(this.sequence.length, 32);
    }
  }
}
