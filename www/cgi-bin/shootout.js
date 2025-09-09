#!/usr/bin/env node

// CGI headers - use single \n instead of \r\n
// console.log("Content-Type: text/html; charset=utf-8");
// console.log(); // Empty line to end headers

// HTML + CSS + JS in one file for simplicity
const html = `Content-Type: text/html; charset=utf-8\r\n\r\n<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>2‑Player Shootout</title>
  <style>
    :root { --bg:#0f1220; --fg:#eaeaf2; --p1:#6ee7ff; --p2:#ff96d5; --bullet:#ffd166; --accent:#8b5cf6; }
    * { box-sizing: border-box; }
    html, body { height: 100%; margin: 0; background: radial-gradient(1200px 800px at 50% 40%, #1a1f35 0%, var(--bg) 60%); font-family: ui-sans-serif, system-ui, -apple-system, Segoe UI, Roboto, Helvetica, Arial, "Apple Color Emoji", "Segoe UI Emoji"; color: var(--fg); }
    .wrap { display:flex; align-items:center; justify-content:center; height:100%; padding: 24px; }
    .panel { width: min(1040px, 96vw); }
    .title { display:flex; align-items:center; justify-content:space-between; gap: 12px; margin-bottom: 12px; }
    .title h1 { margin:0; font-size: clamp(18px, 2vw + 14px, 28px); letter-spacing: .5px; }
    .title .btn { background: #1f2543; border: 1px solid #2c3367; padding: 8px 12px; border-radius: 14px; cursor:pointer; color: var(--fg); }
    .title .btn:hover { filter: brightness(1.1); }
    .hud { display:grid; grid-template-columns: 1fr 1fr; gap: 8px; margin-bottom: 8px; font-size: 14px; }
    .hud .card { background: #13172b; border:1px solid #242a57; border-radius: 16px; padding: 10px 12px; }
    .bar { height: 10px; background: #23284f; border-radius: 999px; overflow: hidden; margin-top: 6px; border: 1px solid #2d3370; }
    .bar .fill { height: 100%; width: 100%; transition: width .2s ease; }
    .p1 .fill { background: linear-gradient(90deg, #0991f1, var(--p1)); }
    .p2 .fill { background: linear-gradient(90deg, #ff3f8e, var(--p2)); }
    #game { width: 100%; aspect-ratio: 16 / 9; display:block; background: repeating-linear-gradient(45deg, #0e1330 0 8px, #0f1437 8px 16px); border: 1px solid #2a2f5f; border-radius: 16px; box-shadow: 0 10px 30px rgba(0,0,0,.4), inset 0 0 0 1px #3a428a; }
    .footer { margin-top: 10px; opacity: .8; font-size: 12px; }
    .kbd { background:#1c2142; border:1px solid #303873; border-radius:8px; padding:2px 6px; font-family: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace; }
    .msg { position:absolute; inset:0; display:none; align-items:center; justify-content:center; }
    .msg.show { display:flex; }
    .msg .box { background:#121633e6; border:1px solid #3a428a; padding: 18px 22px; border-radius: 16px; text-align:center; backdrop-filter: blur(4px); }
    .msg h2 { margin:0 0 6px 0; }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="panel">
      <div class="title">
        <h1>2‑Player Shootout (no server state)</h1>
        <button class="btn" id="resetBtn">Reset</button>
      </div>
      <div class="hud">
        <div class="card">
          <div><strong>Player 1</strong> — Move: <span class="kbd">W A S D</span> • Shoot: <span class="kbd">F</span></div>
          <div class="bar p1"><div class="fill" id="hp1"></div></div>
        </div>
        <div class="card" style="text-align:right">
          <div><strong>Player 2</strong> — Move: <span class="kbd">↑ ← ↓ →</span> • Shoot: <span class="kbd">/</span></div>
          <div class="bar p2"><div class="fill" id="hp2"></div></div>
        </div>
      </div>
      <div style="position:relative">
        <canvas id="game" width="960" height="540"></canvas>
        <div class="msg" id="overlay"><div class="box"><h2 id="winner">Player 1 Wins!</h2><div>Press <span class="kbd">R</span> or click Reset</div></div></div>
      </div>
      <div class="footer">Tip: bullets collide and cancel out. Bump into walls to slide. Game runs fully client‑side; this CGI only serves the page.</div>
    </div>
  </div>

  <script>
  (() => {
    const canvas = document.getElementById('game');
    const ctx = canvas.getContext('2d');
    const overlay = document.getElementById('overlay');
    const winnerEl = document.getElementById('winner');
    const hp1El = document.getElementById('hp1');
    const hp2El = document.getElementById('hp2');
    const resetBtn = document.getElementById('resetBtn');

    const W = () => canvas.width, H = () => canvas.height;

    const state = {
      playing: true,
      players: [
        { x: 80, y: H()/2, vx: 0, vy: 0, dir: {x:1,y:0}, speed: 3.0, color: '#6ee7ff', w: 26, h: 26, hp: 100, cooldown: 0 },
        { x: W()-80, y: H()/2, vx: 0, vy: 0, dir: {x:-1,y:0}, speed: 3.0, color: '#ff96d5', w: 26, h: 26, hp: 100, cooldown: 0 },
      ],
      bullets: [],
      keys: {},
      last: performance.now()
    };

    const keymap = {
      w: 'w', a: 'a', s: 's', d: 'd', f:'f',
      ArrowUp: 'up', ArrowLeft:'left', ArrowDown:'down', ArrowRight:'right', '/':'slash',
      r:'r'
    };

    window.addEventListener('keydown', (e) => {
      const k = keymap[e.key] || keymap[e.key.toLowerCase()];
      if (!k) return;
      state.keys[k] = true;
      if (k === 'r') reset();
      e.preventDefault();
    }, { passive:false });

    window.addEventListener('keyup', (e) => {
      const k = keymap[e.key] || keymap[e.key.toLowerCase()];
      if (!k) return;
      state.keys[k] = false;
      e.preventDefault();
    }, { passive:false });

    resetBtn.addEventListener('click', reset);

    function reset(){
      state.playing = true;
      overlay.classList.remove('show');
      winnerEl.textContent = '';
      state.players[0] = { x: 80, y: H()/2, vx: 0, vy: 0, dir:{x:1,y:0}, speed: 3.0, color: '#6ee7ff', w:26, h:26, hp:100, cooldown:0 };
      state.players[1] = { x: W()-80, y: H()/2, vx: 0, vy: 0, dir:{x:-1,y:0}, speed: 3.0, color: '#ff96d5', w:26, h:26, hp:100, cooldown:0 };
      state.bullets = [];
      hp1El.style.width = '100%';
      hp2El.style.width = '100%';
    }

    function clamp(v, a, b){ return Math.max(a, Math.min(b, v)); }
    function len(x,y){ return Math.hypot(x,y) || 1; }

    function spawnBullet(pIndex){
      const p = state.players[pIndex];
      if (p.cooldown > 0) return;
      const speed = 6.0;
      const dir = {...p.dir};
      const bx = p.x + (p.w/2) + dir.x * (p.w/2 + 6);
      const by = p.y + (p.h/2) + dir.y * (p.h/2 + 6);
      state.bullets.push({ x:bx, y:by, vx:dir.x*speed, vy:dir.y*speed, r:4, owner:pIndex });
      p.cooldown = 180; // ms
    }

    function handleInput(dt){
      const p1 = state.players[0];
      const p2 = state.players[1];

      // P1 movement
      let dx1 = (state.keys['d']?1:0) - (state.keys['a']?1:0);
      let dy1 = (state.keys['s']?1:0) - (state.keys['w']?1:0);
      const l1 = len(dx1,dy1);
      dx1/=l1; dy1/=l1;
      p1.vx = dx1 * p1.speed;
      p1.vy = dy1 * p1.speed;
      if (dx1||dy1) p1.dir = {x:dx1, y:dy1};
      if (state.keys['f']) spawnBullet(0);

      // P2 movement
      let dx2 = (state.keys['right']?1:0) - (state.keys['left']?1:0);
      let dy2 = (state.keys['down']?1:0) - (state.keys['up']?1:0);
      const l2 = len(dx2,dy2);
      dx2/=l2; dy2/=l2;
      p2.vx = dx2 * p2.speed;
      p2.vy = dy2 * p2.speed;
      if (dx2||dy2) p2.dir = {x:dx2, y:dy2};
      if (state.keys['slash']) spawnBullet(1);

      // Cooldowns
      for (const p of state.players){
        p.cooldown = Math.max(0, p.cooldown - dt);
      }
    }

    function update(dt){
      if (!state.playing) return;
      handleInput(dt);

      // Move players with wall bounce/slide
      for (const p of state.players){
        p.x += p.vx;
        p.y += p.vy;
        if (p.x < 8) { p.x = 8; p.vx = 0; }
        if (p.y < 8) { p.y = 8; p.vy = 0; }
        if (p.x + p.w > W()-8) { p.x = W()-8 - p.w; p.vx = 0; }
        if (p.y + p.h > H()-8) { p.y = H()-8 - p.h; p.vy = 0; }
      }

      // Move bullets
      for (const b of state.bullets){
        b.x += b.vx;
        b.y += b.vy;
      }
      // Cull bullets outside
      state.bullets = state.bullets.filter(b => b.x>-20 && b.x<W()+20 && b.y>-20 && b.y<H()+20);

      // Bullet vs bullet collisions
      for (let i=0;i<state.bullets.length;i++){
        for (let j=i+1;j<state.bullets.length;j++){
          const a = state.bullets[i], c = state.bullets[j];
          const dx = a.x - c.x, dy = a.y - c.y;
          if (dx*dx + dy*dy <= (a.r+c.r)*(a.r+c.r)){
            a.dead = true; c.dead = true;
          }
        }
      }
      state.bullets = state.bullets.filter(b => !b.dead);

      // Bullet vs players
      state.bullets.forEach(b => {
        for (let i=0;i<2;i++){
          if (b.owner === i) continue;
          const p = state.players[i];
          if (b.x > p.x && b.x < p.x + p.w && b.y > p.y && b.y < p.y + p.h){
            b.dead = true;
            p.hp = Math.max(0, p.hp - 10);
            if (p.hp === 0){
              state.playing = false;
              overlay.classList.add('show');
              const win = (i === 0) ? 'Player 2 Wins!' : 'Player 1 Wins!';
              winnerEl.textContent = win;
            }
          }
        }
      });
      state.bullets = state.bullets.filter(b => !b.dead);

      // HUD
      hp1El.style.width = state.players[0].hp + '%';
      hp2El.style.width = state.players[1].hp + '%';
    }

    function draw(){
      // Clear
      ctx.clearRect(0,0,W(),H());

      // Border
      ctx.save();
      ctx.strokeStyle = '#2f377a';
      ctx.lineWidth = 2;
      ctx.strokeRect(6,6,W()-12,H()-12);
      ctx.restore();

      // Center line
      ctx.save();
      ctx.globalAlpha = .25;
      ctx.setLineDash([6,6]);
      ctx.beginPath();
      ctx.moveTo(W()/2, 6);
      ctx.lineTo(W()/2, H()-6);
      ctx.strokeStyle = '#6b74cc';
      ctx.lineWidth = 2;
      ctx.stroke();
      ctx.restore();

      // Players
      state.players.forEach(p => {
        // body
        ctx.fillStyle = p.color;
        ctx.fillRect(p.x, p.y, p.w, p.h);

        // direction "nose"
        ctx.save();
        ctx.translate(p.x + p.w/2, p.y + p.h/2);
        const a = Math.atan2(p.dir.y, p.dir.x);
        ctx.rotate(a);
        ctx.fillStyle = '#ffffff';
        ctx.fillRect(p.w/2 - 3, -3, 6, 6);
        ctx.restore();
      });

      // Bullets
      for (const b of state.bullets){
        ctx.beginPath();
        ctx.arc(b.x, b.y, b.r, 0, Math.PI*2);
        ctx.fillStyle = '#ffd166';
        ctx.fill();
      }

      // Particles or extras could go here
    }

    function tick(now){
      const dt = now - state.last;
      state.last = now;
      update(dt);
      draw();
      requestAnimationFrame(tick);
    }

    // Resize canvas to fit CSS size while keeping pixel density sharp
    function fitCanvas(){
      const rect = canvas.getBoundingClientRect();
      const dpr = window.devicePixelRatio || 1;
      const w = Math.round(rect.width * dpr);
      const h = Math.round(rect.height * dpr);
      if (canvas.width !== w || canvas.height !== h){
        canvas.width = w; canvas.height = h;
      }
    }
    window.addEventListener('resize', fitCanvas);
    fitCanvas();
    reset();
    requestAnimationFrame(tick);
  })();
  </script>
</body>
</html>`;

// console.log(html);
process.stdout.write(html);
