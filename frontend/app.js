// ═══════════════════════════════════════════════════════════
  //  UI STATE
  // ═══════════════════════════════════════════════════════════
  const state = {
    playing:     false,
    logVisible:  true,
    progress:    0,       // 0-100
    currentTime: 0,
    totalTime:   0,
    connected:   false,
  };

  // ═══════════════════════════════════════════════════════════
  //  DOM REFS
  // ═══════════════════════════════════════════════════════════
  const $ = id => document.getElementById(id);
  const logPanel      = $('log-panel');
  const cppImage      = $('cpp-image');
  const emptyLabel    = $('empty-label');
  const progressFill  = $('progress-fill');
  const progressThumb = $('progress-thumb');
  const timeCurrent   = $('time-current');
  const timeTotal     = $('time-total');
  const iconPlay      = $('icon-play');
  const iconPause     = $('icon-pause');
  const statusDot     = $('status-dot');
  const btnLogToggle  = $('btn-log-toggle');

  // ═══════════════════════════════════════════════════════════
  //  LOGGING
  // ═══════════════════════════════════════════════════════════
  function ts() {
    const n = new Date();
    return `${String(n.getHours()).padStart(2,'0')}:${String(n.getMinutes()).padStart(2,'0')}:${String(n.getSeconds()).padStart(2,'0')}.${String(n.getMilliseconds()).padStart(3,'0')}`;
  }

  /**
   * Append a log entry.
   * @param {string} msg
   * @param {'log'|'info'|'warn'|'error'} level
   */
  function appendLog(msg, level = 'log') {
    emptyLabel.style.display = 'none';

    const entry = document.createElement('div');
    entry.className = `log-entry ${level}`;
    entry.innerHTML = `<span class="ts">[${ts()}]</span><span class="msg">${escHtml(msg)}</span>`;
    logPanel.appendChild(entry);
    logPanel.scrollTop = logPanel.scrollHeight;
  }

  function escHtml(s) {
    return String(s)
      .replace(/&/g,'&amp;').replace(/</g,'&lt;')
      .replace(/>/g,'&gt;').replace(/"/g,'&quot;');
  }

  // ═══════════════════════════════════════════════════════════
  //  PROGRESS / TIME HELPERS
  // ═══════════════════════════════════════════════════════════
  function fmtTime(sec) {
    const m = Math.floor(sec / 60), s = Math.floor(sec % 60);
    return `${m}:${String(s).padStart(2,'0')}`;
  }

  function setProgress(pct) {
    state.progress = Math.min(100, Math.max(0, pct));
    progressFill.style.width  = state.progress + '%';
    progressThumb.style.left  = state.progress + '%';
  }

  function setTime(current, total) {
    state.currentTime = current;
    state.totalTime   = total;
    timeCurrent.textContent = fmtTime(current);
    timeTotal.textContent   = fmtTime(total);
    if (total > 0) setProgress((current / total) * 100);
  }

  // ═══════════════════════════════════════════════════════════
  //  IMAGE DISPLAY
  // ═══════════════════════════════════════════════════════════
  function showImage(src) {
    cppImage.src = src;
    cppImage.style.display = 'block';
    logPanel.classList.add('has-image');
    appendLog(`Image loaded: ${src}`, 'info');
  }

  function clearImage() {
    cppImage.style.display = 'none';
    cppImage.src = '';
    logPanel.classList.remove('has-image');
  }

  // ═══════════════════════════════════════════════════════════
  //  BACKEND BRIDGE (C++ → JS)
  //  C++ injects calls into this namespace.
  // ═══════════════════════════════════════════════════════════
  window.CppBridge = {

    /** Called by C++ to push a log line */
    log(msg, level = 'log') {
      appendLog(msg, level);
    },

    /** Called by C++ to display an image (base64 data-URI or file:// path) */
    showImage(src) {
      showImage(src);
    },

    /** Called by C++ to update playback time */
    setTime(currentSec, totalSec) {
      setTime(currentSec, totalSec);
    },

    /** Called by C++ to set play/pause state */
    setPlaying(isPlaying) {
      state.playing = isPlaying;
      iconPlay.style.display  = isPlaying ? 'none'  : 'block';
      iconPause.style.display = isPlaying ? 'block' : 'none';
    },

    /** Called by C++ to signal connection ready */
    onConnected() {
      state.connected = true;
      statusDot.classList.add('connected');
      appendLog('Backend connected.', 'info');
    },

    /** Called by C++ to signal disconnection */
    onDisconnected() {
      state.connected = false;
      statusDot.classList.remove('connected');
      appendLog('Backend disconnected.', 'warn');
    },

    /** Called by C++ to clear the image */
    clearImage() {
      clearImage();
    },
  };

  // ═══════════════════════════════════════════════════════════
  //  UI ACTIONS  (JS → C++)
  //  These call into the C++ host via the WebView binding.
  //  Replace window.cppHost.<method>() with your actual
  //  binding name (e.g. webkit.messageHandlers.X.postMessage,
  //  CefV8Value, WebView2 AddHostObjectToScript, etc.)
  // ═══════════════════════════════════════════════════════════
  function callCpp(method, ...args) {
    // ── WebView2 (Windows)  ──────────────────────────────────
    if (window.chrome?.webview?.hostObjects?.cppHost) {
      window.chrome.webview.hostObjects.cppHost[method](...args);
      return;
    }
    // ── WKWebView (macOS/iOS) ────────────────────────────────
    if (window.webkit?.messageHandlers?.cppHost) {
      window.webkit.messageHandlers.cppHost.postMessage({ method, args });
      return;
    }
    // ── CEF / custom binding ─────────────────────────────────
    if (window.cppHost) {
      if (typeof window.cppHost[method] === 'function') {
        window.cppHost[method](...args);
        return;
      }
    }
    // ── Fallback: log locally ────────────────────────────────
    appendLog(`[no backend] ${method}(${args.map(JSON.stringify).join(', ')})`, 'warn');
  }

  window.UI = {
    togglePlay() {
      callCpp('togglePlay');
    },
    prev() {
      callCpp('prev');
      appendLog('← Previous', 'log');
    },
    next() {
      callCpp('next');
      appendLog('→ Next', 'log');
    },
    seek(e) {
      const rect = e.currentTarget.getBoundingClientRect();
      const pct  = ((e.clientX - rect.left) / rect.width) * 100;
      setProgress(pct);
      const seekSec = (pct / 100) * state.totalTime;
      callCpp('seek', seekSec);
    },
    requestImage() {
      callCpp('openImage');
      appendLog('Requesting image from backend…', 'info');
    },
    toggleLog() {
      state.logVisible = !state.logVisible;
      logPanel.style.display = state.logVisible ? 'block' : 'none';
      btnLogToggle.classList.toggle('active', state.logVisible);
    },
  };

  // ═══════════════════════════════════════════════════════════
  //  INIT
  // ═══════════════════════════════════════════════════════════
  appendLog('UI ready. Waiting for backend…', 'info');