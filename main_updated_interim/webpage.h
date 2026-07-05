#pragma once
#include <pgmspace.h>

const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
  <meta name="apple-mobile-web-app-capable" content="yes" />
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent" />
  <meta name="apple-mobile-web-app-title" content="ESP32" />
  <meta name="theme-color" content="#0a0c0f" />
  <title>ESP32 Control</title>

  <script>
    const manifest = {
      name: "ESP32 Control Panel",
      short_name: "ESP32",
      start_url: ".",
      display: "standalone",
      background_color: "#0a0c0f",
      theme_color: "#0a0c0f",
      icons: [
        { src: "data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 192 192'%3E%3Crect width='192' height='192' fill='%230a0c0f'/%3E%3Crect x='40' y='40' width='112' height='112' rx='8' fill='none' stroke='%2300ffa0' stroke-width='4'/%3E%3Ccircle cx='96' cy='96' r='24' fill='%2300ffa0'/%3E%3C/svg%3E", sizes: "192x192", type: "image/svg+xml" }
      ]
    };
    const blob = new Blob([JSON.stringify(manifest)], { type: "application/json" });
    const link = document.createElement("link");
    link.rel = "manifest";
    link.href = URL.createObjectURL(blob);
    document.head.appendChild(link);
  </script>

  <link rel="preconnect" href="https://fonts.googleapis.com" />
  <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin />
  <link href="https://fonts.googleapis.com/css2?family=Share+Tech+Mono&family=Rajdhani:wght@400;600;700&display=swap" rel="stylesheet" />

  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

    :root {
      --green:  #00ffa0;
      --blue:   #00c8ff;
      --red:    #ff4060;
      --yellow: #ffc040;
      --purple: #c084fc;
      --pink:   #ff70c0;
      --bg:     #0a0c0f;
      --panel:  #0e1117;
      --border: #1e2a3a;
      --text:   #e0e6f0;
      --dim:    #4a6080;
      --dimmer: #2a3a50;
      --safe-top:    env(safe-area-inset-top, 0px);
      --safe-bottom: env(safe-area-inset-bottom, 0px);
    }

    html, body {
      height: 100%;
      background: var(--bg);
      color: var(--text);
      font-family: 'Rajdhani', sans-serif;
      -webkit-tap-highlight-color: transparent;
      -webkit-text-size-adjust: 100%;
      overflow: hidden;
    }

    #app {
      height: 100dvh;
      display: flex; flex-direction: column;
      padding-top: var(--safe-top);
      padding-bottom: var(--safe-bottom);
      position: relative; overflow: hidden;
    }

    #app::before {
      content: '';
      position: fixed; inset: 0;
      background:
        repeating-linear-gradient(0deg, transparent, transparent 39px, rgba(0,255,160,0.025) 40px),
        repeating-linear-gradient(90deg, transparent, transparent 39px, rgba(0,255,160,0.025) 40px);
      pointer-events: none; z-index: 0;
    }

    .corner { position: fixed; width: 40px; height: 40px; opacity: 0.35; z-index: 1; pointer-events: none; }
    .corner-tl { top: calc(var(--safe-top) + 12px);     left: 12px;  border-top: 2px solid var(--green); border-left: 2px solid var(--green); }
    .corner-tr { top: calc(var(--safe-top) + 12px);     right: 12px; border-top: 2px solid var(--green); border-right: 2px solid var(--green); }
    .corner-bl { bottom: calc(var(--safe-bottom)+12px); left: 12px;  border-bottom: 2px solid var(--green); border-left: 2px solid var(--green); }
    .corner-br { bottom: calc(var(--safe-bottom)+12px); right: 12px; border-bottom: 2px solid var(--green); border-right: 2px solid var(--green); }

    .screen { display: none; flex-direction: column; height: 100%; }
    .screen.active { display: flex; }

    .msg-portal-btn {
      width: 100%; padding: 16px;
      background: rgba(255,112,192,0.06); border: 1px solid rgba(255,112,192,0.25);
      color: var(--pink); font-family: 'Rajdhani', sans-serif;
      font-size: 13px; font-weight: 700; letter-spacing: 2px;
      text-transform: uppercase; cursor: pointer;
      display: flex; align-items: center; justify-content: center; gap: 10px;
      transition: all 0.12s;
    }
    .msg-portal-btn:active { background: rgba(255,112,192,0.14); border-color: var(--pink); }
    .msg-portal-icon { font-size: 16px; }

    #portalScreen {
      align-items: center; justify-content: center;
      padding: 40px 24px; gap: 0;
    }

    .portal-back {
      position: absolute; top: calc(var(--safe-top) + 16px); left: 16px;
      background: transparent; border: 1px solid var(--border);
      color: var(--dim); font-size: 18px; width: 40px; height: 40px;
      cursor: pointer; display: flex; align-items: center; justify-content: center;
      z-index: 10; transition: all 0.15s;
    }
    .portal-back:active { border-color: var(--green); color: var(--green); }

    .portal-title {
      font-size: 20px; font-weight: 700; letter-spacing: 3px;
      text-transform: uppercase; margin-bottom: 6px; text-align: center;
    }
    .portal-sub {
      font-family: 'Share Tech Mono', monospace; font-size: 9px;
      color: var(--dim); letter-spacing: 1px; margin-bottom: 32px; text-align: center;
    }

    .portal-field { width: 100%; max-width: 320px; margin-bottom: 14px; }
    .portal-field label {
      display: block; font-family: 'Share Tech Mono', monospace;
      font-size: 9px; color: var(--dim); letter-spacing: 2px;
      text-transform: uppercase; margin-bottom: 6px;
    }
    .portal-field input, .portal-field textarea {
      width: 100%; padding: 14px;
      background: #080b0f; border: 1px solid var(--border);
      color: var(--text); font-family: 'Share Tech Mono', monospace;
      font-size: 14px; outline: none; border-radius: 0;
      -webkit-appearance: none; letter-spacing: 0.5px;
      transition: border-color 0.15s; resize: none;
    }
    .portal-field input:focus, .portal-field textarea:focus {
      border-color: rgba(255,112,192,0.4);
    }
    .portal-field textarea { min-height: 90px; }

    .portal-char-count {
      font-family: 'Share Tech Mono', monospace; font-size: 9px;
      color: var(--dimmer); text-align: right; margin-top: 4px;
    }
    .portal-char-count.over { color: var(--red); }

    .portal-send-btn {
      width: 100%; max-width: 320px; padding: 16px;
      background: rgba(255,112,192,0.08); border: 1px solid rgba(255,112,192,0.35);
      color: var(--pink); font-family: 'Rajdhani', sans-serif;
      font-size: 14px; font-weight: 700; letter-spacing: 3px;
      text-transform: uppercase; cursor: pointer; margin-top: 6px;
      transition: all 0.12s;
    }
    .portal-send-btn:active { background: rgba(255,112,192,0.18); }
    .portal-send-btn:disabled { opacity: 0.3; pointer-events: none; }

    .portal-status {
      font-family: 'Share Tech Mono', monospace; font-size: 10px;
      letter-spacing: 1px; margin-top: 14px; min-height: 16px;
      text-align: center; max-width: 320px; width: 100%;
    }
    .portal-status.ok   { color: var(--green); }
    .portal-status.err  { color: var(--red); }

    .header {
      position: relative; z-index: 10;
      padding: 16px 20px 14px;
      border-bottom: 1px solid var(--border);
      display: flex; align-items: center; gap: 14px;
      background: var(--panel); flex-shrink: 0;
    }
    .header::after {
      content: ''; position: absolute; top: 0; left: 0; right: 0; height: 2px;
      background: linear-gradient(90deg, transparent, var(--green), var(--blue), transparent);
    }

    .status-dot {
      width: 9px; height: 9px; border-radius: 50%;
      background: var(--green);
      box-shadow: 0 0 8px var(--green), 0 0 16px rgba(0,255,160,0.5);
      animation: pulse 2s ease-in-out infinite; flex-shrink: 0;
    }
    .status-dot.busy  { background: var(--blue);   box-shadow: 0 0 8px var(--blue);  animation: pulse 0.6s ease-in-out infinite; }
    .status-dot.error { background: var(--red);    box-shadow: 0 0 8px var(--red);   animation: none; }
    .status-dot.off   { background: var(--dimmer); box-shadow: none;                 animation: none; }
    @keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.3} }

    .header-text { flex: 1; min-width: 0; }
    .header-text h1 { font-size: 18px; font-weight: 700; letter-spacing: 3px; text-transform: uppercase; }
    .header-text p {
      font-family: 'Share Tech Mono', monospace;
      font-size: 9px; color: var(--dim); letter-spacing: 1px; margin-top: 1px;
      white-space: nowrap; overflow: hidden; text-overflow: ellipsis;
    }

    .header-right { display: flex; gap: 8px; flex-shrink: 0; }

    .inbox-btn {
      width: 40px; height: 40px; position: relative;
      background: transparent; border: 1px solid var(--border);
      color: var(--dim); font-size: 16px; cursor: pointer;
      display: flex; align-items: center; justify-content: center;
      transition: all 0.15s;
    }
    .inbox-btn:active { background: rgba(255,112,192,0.05); border-color: var(--pink); color: var(--pink); }
    .inbox-btn.has-mail { border-color: rgba(255,112,192,0.4); color: var(--pink); }
    .inbox-badge {
      position: absolute; top: -5px; right: -5px;
      min-width: 16px; height: 16px; padding: 0 3px;
      background: var(--pink); color: #000;
      font-family: 'Share Tech Mono', monospace; font-size: 9px; font-weight: 700;
      border-radius: 8px; display: flex; align-items: center; justify-content: center;
      display: none;
    }
    .inbox-btn.has-mail .inbox-badge { display: flex; }

    .settings-btn {
      width: 40px; height: 40px;
      background: transparent; border: 1px solid var(--border);
      color: var(--dim); font-size: 18px; cursor: pointer;
      display: flex; align-items: center; justify-content: center;
      transition: all 0.15s; flex-shrink: 0;
    }
    .settings-btn:active { background: rgba(0,255,160,0.05); border-color: var(--green); color: var(--green); }

    .body {
      flex: 1; overflow-y: auto; -webkit-overflow-scrolling: touch;
      position: relative; z-index: 2;
      padding: 20px 16px;
      display: flex; flex-direction: column; gap: 22px;
    }

    .section-label {
      font-size: 9px; letter-spacing: 3px; text-transform: uppercase;
      color: var(--dim); margin-bottom: 10px;
      display: flex; align-items: center; gap: 8px;
    }
    .section-label::after { content: ''; flex: 1; height: 1px; background: var(--border); }

    .btn-row { display: grid; grid-template-columns: repeat(3, 1fr); gap: 8px; margin-bottom: 8px; }

    .ctrl-btn {
      position: relative; padding: 16px 10px;
      background: #111820; border: 1px solid var(--border);
      color: var(--text); font-family: 'Rajdhani', sans-serif; font-weight: 700;
      letter-spacing: 1px; text-transform: uppercase; cursor: pointer;
      display: flex; flex-direction: column; align-items: flex-start; gap: 4px;
      clip-path: polygon(0 0, calc(100% - 8px) 0, 100% 8px, 100% 100%, 8px 100%, 0 calc(100% - 8px));
      -webkit-tap-highlight-color: transparent;
      transition: all 0.12s; width: 100%; text-align: left;
      overflow: hidden; min-height: 72px;
    }
    .ctrl-btn::before { content: ''; position: absolute; inset: 0; opacity: 0; transition: opacity 0.15s; }
    .btn-a::before   { background: linear-gradient(135deg, rgba(0,255,160,0.08), transparent); }
    .btn-b::before   { background: linear-gradient(135deg, rgba(0,200,255,0.08), transparent); }
    .btn-df::before  { background: linear-gradient(135deg, rgba(255,192,64,0.08), transparent); }
    .btn-clk::before { background: linear-gradient(135deg, rgba(192,132,252,0.08), transparent); }
    .btn-spt::before { background: linear-gradient(135deg, rgba(255,64,96,0.08), transparent); }
    .ctrl-btn:active::before { opacity: 1; }
    .ctrl-btn:active { transform: scale(0.97); }
    .ctrl-btn.loading { opacity: 0.55; pointer-events: none; }

    .btn-id { font-family: 'Share Tech Mono', monospace; font-size: 9px; letter-spacing: 1px; }
    .btn-a   .btn-id { color: var(--green); }
    .btn-b   .btn-id { color: var(--blue); }
    .btn-df  .btn-id { color: var(--yellow); }
    .btn-clk .btn-id { color: var(--purple); }
    .btn-spt .btn-id { color: var(--red); }
    .btn-label { font-size: 14px; line-height: 1.2; }

    .color-panel {
      background: #080b0f; border: 1px solid var(--border);
      border-top: none; padding: 14px;
    }
    .color-panel-label {
      font-family: 'Share Tech Mono', monospace; font-size: 9px;
      color: var(--dim); letter-spacing: 2px; text-transform: uppercase; margin-bottom: 10px;
    }
    .color-btn-row { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 8px; }
    .color-btn {
      padding: 12px 6px;
      background: transparent; border: 1px solid;
      font-family: 'Rajdhani', sans-serif; font-size: 14px; font-weight: 700;
      letter-spacing: 2px; text-transform: uppercase; cursor: pointer;
      transition: all 0.12s; opacity: 0.6;
    }
    .color-btn:active { opacity: 1; transform: scale(0.97); }
    .color-btn.active { opacity: 1; }

    .power-btn {
      position: relative; padding: 14px 16px;
      background: rgba(255,64,96,0.06); border: 1px solid rgba(255,64,96,0.25);
      color: var(--red); font-family: 'Rajdhani', sans-serif; font-size: 14px; font-weight: 700;
      letter-spacing: 2px; text-transform: uppercase; cursor: pointer; width: 100%;
      display: flex; align-items: center; justify-content: center; gap: 10px;
      -webkit-tap-highlight-color: transparent; transition: all 0.12s;
    }
    .power-btn:active   { background: rgba(255,64,96,0.14); border-color: var(--red); }
    .power-btn:disabled { opacity: 0.35; pointer-events: none; }
    .power-icon { font-size: 16px; }

    .msg-row { display: flex; }
    .msg-input {
      flex: 1; padding: 16px 14px;
      background: #080b0f; border: 1px solid var(--border); border-right: none;
      color: var(--text); font-family: 'Share Tech Mono', monospace; font-size: 14px;
      outline: none; -webkit-appearance: none; border-radius: 0;
      letter-spacing: 0.5px; transition: border-color 0.15s; min-width: 0;
    }
    .msg-input::placeholder { color: var(--dimmer); }
    .msg-input:focus { border-color: rgba(0,255,160,0.3); }
    .send-btn {
      padding: 16px 20px;
      background: rgba(0,255,160,0.08); border: 1px solid rgba(0,255,160,0.3);
      color: var(--green); font-family: 'Rajdhani', sans-serif;
      font-size: 13px; font-weight: 700; letter-spacing: 2px; text-transform: uppercase;
      cursor: pointer; white-space: nowrap; flex-shrink: 0; transition: all 0.12s;
    }
    .send-btn:active   { background: rgba(0,255,160,0.18); }
    .send-btn:disabled { opacity: 0.3; pointer-events: none; }

    .log-box {
      background: #080b0f; border: 1px solid var(--border);
      padding: 12px; max-height: 180px;
      overflow-y: auto; -webkit-overflow-scrolling: touch;
      font-family: 'Share Tech Mono', monospace; font-size: 11px;
    }
    .log-entry {
      display: flex; gap: 10px; padding: 4px 0;
      border-bottom: 1px solid #0a0d11; animation: fadeIn 0.2s ease;
    }
    @keyframes fadeIn { from{opacity:0;transform:translateY(-3px)} to{opacity:1;transform:none} }
    .log-time  { color: var(--dimmer); flex-shrink: 0; }
    .log-ok    { color: var(--green); }
    .log-err   { color: var(--red); }
    .log-info  { color: var(--blue); }
    .log-warn  { color: var(--yellow); }
    .log-empty { color: var(--dimmer); font-style: italic; }

    .ip-tag {
      font-family: 'Share Tech Mono', monospace; font-size: 9px;
      color: var(--dimmer); text-align: center; padding: 10px 0 4px; letter-spacing: 1px;
    }

    .spinner {
      display: inline-block; width: 9px; height: 9px;
      border: 1.5px solid rgba(0,255,160,0.2); border-top-color: var(--green);
      border-radius: 50%; animation: spin 0.6s linear infinite;
      vertical-align: middle; margin-right: 5px;
    }
    .spinner.blue   { border-color: rgba(0,200,255,0.2);   border-top-color: var(--blue); }
    .spinner.yellow { border-color: rgba(255,192,64,0.2);  border-top-color: var(--yellow); }
    .spinner.purple { border-color: rgba(192,132,252,0.2); border-top-color: var(--purple); }
    .spinner.red    { border-color: rgba(255,64,96,0.2);   border-top-color: var(--red); }
    @keyframes spin { to{transform:rotate(360deg)} }

    .inbox-overlay {
      position: fixed; inset: 0; background: rgba(0,0,0,0.88);
      z-index: 200; display: flex; align-items: flex-end;
      opacity: 0; pointer-events: none; transition: opacity 0.2s;
    }
    .inbox-overlay.open { opacity: 1; pointer-events: all; }
    .inbox-sheet {
      width: 100%; background: var(--panel); border-top: 1px solid rgba(255,112,192,0.3);
      padding: 0 20px calc(var(--safe-bottom) + 24px);
      transform: translateY(100%);
      transition: transform 0.25s cubic-bezier(0.32,0.72,0,1);
      max-height: 80dvh; overflow-y: auto; -webkit-overflow-scrolling: touch;
    }
    .inbox-overlay.open .inbox-sheet { transform: translateY(0); }
    .sheet-handle { width: 36px; height: 4px; background: var(--border); border-radius: 2px; margin: 12px auto 20px; }
    .inbox-sheet-title {
      font-size: 11px; letter-spacing: 3px; text-transform: uppercase;
      color: var(--pink); margin-bottom: 18px;
      display: flex; align-items: center; gap: 10px;
    }
    .inbox-sheet-title::after { content: ''; flex: 1; height: 1px; background: rgba(255,112,192,0.2); }

    .inbox-list { display: flex; flex-direction: column; gap: 8px; margin-bottom: 16px; }
    .inbox-item {
      background: #080b0f; border: 1px solid var(--border);
      padding: 14px; cursor: pointer; transition: all 0.12s;
      display: flex; align-items: center; justify-content: space-between; gap: 12px;
    }
    .inbox-item:active { border-color: rgba(255,112,192,0.4); background: rgba(255,112,192,0.04); }
    .inbox-item.viewed { opacity: 0.45; }
    .inbox-item-left { flex: 1; min-width: 0; }
    .inbox-item-name {
      font-weight: 700; font-size: 15px; letter-spacing: 1px;
      color: var(--text); margin-bottom: 2px;
    }
    .inbox-item-preview {
      font-family: 'Share Tech Mono', monospace; font-size: 9px;
      color: var(--dim); letter-spacing: 0.5px;
      white-space: nowrap; overflow: hidden; text-overflow: ellipsis;
    }
    .inbox-item-time {
      font-family: 'Share Tech Mono', monospace; font-size: 9px;
      color: var(--dimmer); flex-shrink: 0;
    }
    .inbox-item-arrow { color: var(--pink); font-size: 14px; flex-shrink: 0; }

    .inbox-empty {
      font-family: 'Share Tech Mono', monospace; font-size: 11px;
      color: var(--dimmer); text-align: center; padding: 30px 0; font-style: italic;
    }

    .inbox-close-btn {
      position: absolute; top: 0; left: 0; right: 0; height: 20%;
      cursor: pointer;
    }

    .overlay {
      position: fixed; inset: 0; background: rgba(0,0,0,0.85);
      z-index: 100; display: flex; align-items: flex-end;
      opacity: 0; pointer-events: none; transition: opacity 0.2s;
    }
    .overlay.open { opacity: 1; pointer-events: all; }
    .settings-sheet {
      width: 100%; background: var(--panel); border-top: 1px solid var(--border);
      padding: 0 20px calc(var(--safe-bottom) + 24px);
      transform: translateY(100%);
      transition: transform 0.25s cubic-bezier(0.32,0.72,0,1);
      max-height: 90dvh; overflow-y: auto; -webkit-overflow-scrolling: touch;
    }
    .overlay.open .settings-sheet { transform: translateY(0); }
    .sheet-title { font-size: 11px; letter-spacing: 3px; text-transform: uppercase; color: var(--dim); margin-bottom: 20px; }
    .settings-group { margin-bottom: 24px; }
    .settings-group-label { font-size: 9px; letter-spacing: 2px; text-transform: uppercase; color: var(--dimmer); margin-bottom: 10px; padding-bottom: 6px; border-bottom: 1px solid var(--border); }
    .settings-field { margin-bottom: 14px; }
    .settings-field label { display: block; font-family: 'Share Tech Mono', monospace; font-size: 10px; color: var(--dim); letter-spacing: 1px; margin-bottom: 6px; }
    .settings-field input { width: 100%; padding: 14px; background: #080b0f; border: 1px solid var(--border); color: var(--text); font-family: 'Share Tech Mono', monospace; font-size: 14px; outline: none; -webkit-appearance: none; border-radius: 0; letter-spacing: 0.5px; }
    .settings-field input:focus { border-color: rgba(0,255,160,0.3); }
    .time-row { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .toggle-row { display: flex; align-items: center; justify-content: space-between; padding: 14px 0; border-bottom: 1px solid var(--border); margin-bottom: 14px; }
    .toggle-label { font-family: 'Share Tech Mono', monospace; font-size: 11px; color: var(--text); letter-spacing: 0.5px; }
    .toggle-sub { font-size: 9px; color: var(--dim); margin-top: 2px; }
    .toggle { position: relative; width: 44px; height: 26px; flex-shrink: 0; }
    .toggle input { opacity: 0; width: 0; height: 0; }
    .toggle-track { position: absolute; inset: 0; background: var(--dimmer); border-radius: 13px; cursor: pointer; transition: background 0.2s; }
    .toggle-track::after { content: ''; position: absolute; width: 20px; height: 20px; border-radius: 50%; background: var(--text); top: 3px; left: 3px; transition: transform 0.2s; }
    .toggle input:checked + .toggle-track { background: var(--green); }
    .toggle input:checked + .toggle-track::after { transform: translateX(18px); }
    .save-btn { width: 100%; padding: 16px; background: rgba(0,255,160,0.1); border: 1px solid rgba(0,255,160,0.4); color: var(--green); font-family: 'Rajdhani', sans-serif; font-size: 14px; font-weight: 700; letter-spacing: 3px; text-transform: uppercase; cursor: pointer; margin-top: 4px; }
    .save-btn:active { background: rgba(0,255,160,0.2); }
    .close-overlay { position: absolute; inset: 0; bottom: auto; height: 15%; }

    #sportsPanel {
      display: none; flex-direction: column; gap: 8px;
      margin-top: 10px;
    }
    #sportsPanel.open { display: flex; }
    .sports-panel-header { display: flex; justify-content: space-between; align-items: center; }
    .sports-panel-title { font-family: 'Share Tech Mono', monospace; font-size: 9px; color: var(--dim); letter-spacing: 2px; text-transform: uppercase; }
    .sports-panel-status { font-family: 'Share Tech Mono', monospace; font-size: 9px; color: var(--dimmer); letter-spacing: 1px; }
    .sports-refresh-inline { background: transparent; border: 1px solid var(--border); color: var(--dim); font-size: 13px; width: 28px; height: 28px; cursor: pointer; display: flex; align-items: center; justify-content: center; transition: all 0.15s; flex-shrink: 0; }
    .sports-refresh-inline:active { border-color: var(--green); color: var(--green); }
    .sports-refresh-inline.spinning { animation: spin 0.8s linear infinite; pointer-events: none; }
    .sports-section-label { font-family: 'Share Tech Mono', monospace; font-size: 9px; color: var(--dim); letter-spacing: 2px; text-transform: uppercase; margin-top: 4px; margin-bottom: 2px; }
    .score-bug { background: #080d16; border: 1px solid var(--border); overflow: hidden; clip-path: polygon(0 0, calc(100% - 8px) 0, 100% 8px, 100% 100%, 8px 100%, 0 calc(100% - 8px)); }
    .score-bug-header { display: flex; justify-content: space-between; align-items: center; padding: 6px 12px; background: #0c1628; border-bottom: 1px solid #1e2a3a; }
    .score-bug-sport { font-family: 'Share Tech Mono', monospace; font-size: 11px; color: #64c8ff; letter-spacing: 2px; text-transform: uppercase; }
    .score-bug-status { font-family: 'Share Tech Mono', monospace; font-size: 11px; letter-spacing: 1px; }
    .status-live { color: #ffd840; animation: pulse 1.4s ease-in-out infinite; }
    .status-fin  { color: #606878; }
    .status-pre  { color: #64c8ff; }
    .score-bug-row { display: flex; align-items: center; padding: 10px 12px; gap: 10px; border-bottom: 1px solid #0e1825; }
    .score-bug-row:last-child { border-bottom: none; }
    .score-bug-accent { width: 3px; height: 28px; flex-shrink: 0; border-radius: 1px; }
    .accent-away { background: #5090ff; }
    .accent-home { background: #ffc832; }
    .score-bug-team { font-family: 'Share Tech Mono', monospace; font-size: 16px; font-weight: 700; letter-spacing: 2px; color: #b4bdd0; flex: 1; text-transform: uppercase; }
    .score-bug-team.winner { color: #ffffff; }
    .score-bug-score { font-family: 'Share Tech Mono', monospace; font-size: 22px; font-weight: 700; color: #ffffff; min-width: 32px; text-align: right; }
    .score-bug-score.loser { color: #4a5870; }
    .next-game-card { background: #080d16; border: 1px solid var(--border); padding: 10px 12px; display: flex; flex-direction: column; gap: 3px; }
    .next-game-top { display: flex; justify-content: space-between; align-items: center; }
    .next-game-sport { font-family: 'Share Tech Mono', monospace; font-size: 9px; color: #64c8ff; letter-spacing: 2px; text-transform: uppercase; }
    .next-game-label { font-family: 'Share Tech Mono', monospace; font-size: 9px; color: var(--dimmer); letter-spacing: 2px; }
    .next-game-teams { font-size: 14px; font-weight: 700; letter-spacing: 1px; color: var(--text); }
    .next-game-date { font-family: 'Share Tech Mono', monospace; font-size: 11px; color: #ffc832; letter-spacing: 1px; }
    .last-game-card { background: #080d16; border: 1px solid var(--border); padding: 8px 12px; display: flex; align-items: center; gap: 10px; }
    .last-game-sport { font-family: 'Share Tech Mono', monospace; font-size: 9px; color: var(--dimmer); letter-spacing: 2px; text-transform: uppercase; min-width: 44px; }
    .last-game-result { flex: 1; font-family: 'Share Tech Mono', monospace; font-size: 12px; color: var(--dim); letter-spacing: 1px; }
    .last-game-result .w { color: var(--text); font-weight: 700; }
    .sports-empty { font-family: 'Share Tech Mono', monospace; font-size: 11px; color: var(--dimmer); letter-spacing: 1px; text-align: center; padding: 16px 0; }
    .loading-dots { font-family: 'Share Tech Mono', monospace; font-size: 11px; color: var(--dimmer); animation: blink 1.2s step-start infinite; }
    @keyframes blink { 0%,100%{opacity:1} 50%{opacity:0.2} }

    /* Priority List Styles */
    .prio-list { display: flex; flex-direction: column; gap: 6px; }
    .prio-item { display: flex; justify-content: space-between; align-items: center; background: #080b0f; border: 1px solid var(--border); padding: 10px 14px; font-family: 'Share Tech Mono', monospace; font-size: 13px; }
    .prio-controls { display: flex; gap: 8px; }
    .prio-btn { background: rgba(0,255,160,0.1); border: 1px solid var(--green); color: var(--green); width: 28px; height: 28px; border-radius: 4px; display: flex; align-items: center; justify-content: center; cursor: pointer; }
    .prio-btn:active { background: rgba(0,255,160,0.3); }
    .prio-btn:disabled { opacity: 0.3; pointer-events: none; border-color: var(--dim); color: var(--dim); background: transparent; }

  </style>
</head>
<body>
<div id="app">
  <div class="corner corner-tl"></div>
  <div class="corner corner-tr"></div>
  <div class="corner corner-bl"></div>
  <div class="corner corner-br"></div>

  <div id="portalScreen" class="screen">
    <button class="portal-back" onclick="showMain()">&#8592;</button>
    <div class="portal-title">Leave a Message</div>
    <div class="portal-sub">ETHAN WILL SEE THIS ON HIS LED BOARD</div>

    <div class="portal-field">
      <label>Your Name</label>
      <input type="text" id="portalName" placeholder="Your name..."
        autocomplete="name" autocorrect="off" autocapitalize="words"
        onkeydown="if(event.key==='Enter') document.getElementById('portalMsg').focus()" />
    </div>

    <div class="portal-field">
      <label>Message</label>
      <textarea id="portalMsg" placeholder="Type your message..."
        maxlength="120" oninput="updateCharCount()"
        onkeydown="if(event.key==='Enter' && event.ctrlKey) sendPortalMsg()"></textarea>
      <div class="portal-char-count" id="charCount">0 / 120</div>
    </div>

    <div class="portal-status" id="portalStatus"></div>
    <button class="portal-send-btn" id="portalSendBtn" onclick="sendPortalMsg()">
      Send Message &#10148;
    </button>
  </div>

  <div id="mainScreen" class="screen active">

    <div class="header">
      <div class="status-dot" id="statusDot"></div>
      <div class="header-text">
        <h1>ESP32 Control</h1>
        <p>DEVICE INTERFACE · REMOTE EXECUTION PANEL</p>
      </div>
      <div class="header-right">
        <button class="inbox-btn" id="inboxBtn" onclick="openInbox()" aria-label="Inbox">
          ✉<span class="inbox-badge" id="inboxBadge">0</span>
        </button>
        <button class="settings-btn" onclick="openSettings()" aria-label="Settings">&#9881;</button>
      </div>
    </div>

    <div class="body">
      <div>
        <div class="section-label">Programs</div>
        <div class="btn-row">
          <button class="ctrl-btn btn-a" id="btnA" onclick="runA()">
            <span class="btn-id" id="btnAId">0x01</span><span class="btn-label">Weather</span>
          </button>
          <button class="ctrl-btn btn-b" id="btnB" onclick="runB()">
            <span class="btn-id" id="btnBId">0x02</span><span class="btn-label">Flight Data</span>
          </button>
          <button class="ctrl-btn btn-df" id="btnDF" onclick="runDefault()">
            <span class="btn-id" id="btnDFId">AUTO</span><span class="btn-label">Default</span>
          </button>
          <button class="ctrl-btn btn-clk" id="btnCLK" onclick="runClock()">
            <span class="btn-id" id="btnCLKId">0x04</span><span class="btn-label">Clock</span>
          </button>
          <button class="ctrl-btn btn-spt" id="btnSpt" onclick="runSports()">
            <span class="btn-id" id="btnSptId">0x05</span><span class="btn-label">Sports</span>
          </button>
        </div>

        <div id="sportsPanel">
          <div class="sports-panel-header">
            <div>
              <span class="sports-panel-title">&#127942; Sports</span>
              <span class="sports-panel-status" id="sportsPanelStatus"></span>
            </div>
            <button class="sports-refresh-inline" id="sportsRefreshBtn"
                    onclick="refreshSports()" title="Refresh">&#8635;</button>
          </div>
          <div id="sportsBody">
            <div class="sports-empty">Press Sports to load scores...</div>
          </div>
        </div>

        <div class="color-panel">
          <div class="color-panel-label">Clock Color</div>
          <div class="color-btn-row">
            <button class="color-btn" id="clr-red"   onclick="setClockColor('ff0000')" style="color:#ff4060;border-color:#ff4060;">Red</button>
            <button class="color-btn" id="clr-blue"  onclick="setClockColor('0080ff')" style="color:#0080ff;border-color:#0080ff;">Blue</button>
            <button class="color-btn" id="clr-white" onclick="setClockColor('ffffff')" style="color:#ffffff;border-color:#ffffff;">White</button>
          </div>
        </div>
      </div>

      <div>
        <div class="section-label">Power</div>
        <button class="power-btn" id="powerBtn" onclick="boardOff()">
          <span class="power-icon">&#9211;</span><span>Turn Board Off</span>
        </button>
      </div>

      <div>
        <div class="section-label">Display Output</div>
        <div class="msg-row">
          <input class="msg-input" id="msgInput" type="text"
            placeholder="Enter message to display..."
            autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false"
            onkeydown="if(event.key==='Enter') sendMsg()" />
          <button class="send-btn" id="sendBtn" onclick="sendMsg()">Send</button>
        </div>
      </div>

      <div>
        <div class="section-label">Guest Portal</div>
        <button class="msg-portal-btn" onclick="showPortal()">
          <span class="msg-portal-icon">✉</span><span>Send Message to Inbox</span>
        </button>
      </div>

      <div>
        <div class="section-label">Activity Log</div>
        <div class="log-box" id="logBox">
          <div class="log-entry"><span class="log-empty">No activity yet...</span></div>
        </div>
      </div>
      <div class="ip-tag">ESP32 HOSTED</div>
    </div>

    <div class="inbox-overlay" id="inboxOverlay">
      <div class="inbox-close-btn" onclick="closeInbox()"></div>
      <div class="inbox-sheet">
        <div class="sheet-handle"></div>
        <div class="inbox-sheet-title">Inbox</div>
        <div class="inbox-list" id="inboxList"></div>
      </div>
    </div>

    <div class="overlay" id="settingsOverlay">
      <div class="close-overlay" onclick="closeSettings()"></div>
      <div class="settings-sheet">
        <div class="sheet-handle"></div>
        <div class="sheet-title">Settings</div>
        
        <div class="settings-group">
          <div class="settings-group-label">Sleep Schedule</div>
          <div class="toggle-row">
            <div>
              <div class="toggle-label">Auto Sleep</div>
              <div class="toggle-sub">Board turns off and on automatically</div>
            </div>
            <label class="toggle">
              <input type="checkbox" id="sleepEnabled" />
              <span class="toggle-track"></span>
            </label>
          </div>
          <div class="time-row">
            <div class="settings-field">
              <label>SLEEP TIME</label>
              <input type="time" id="sleepTime" value="23:00" />
            </div>
            <div class="settings-field">
              <label>WAKE TIME</label>
              <input type="time" id="wakeTime" value="07:00" />
            </div>
          </div>
        </div>

        <div class="settings-group">
          <div class="settings-group-label">Sports Priority (Top = First shown)</div>
          <div class="prio-list" id="prioList">
            </div>
        </div>

        <button class="save-btn" onclick="saveSettings()">Save &amp; Close</button>
      </div>
    </div>
  </div>
</div>

<script>
  function showScreen(id) {
    document.querySelectorAll('.screen').forEach(function(s) { s.classList.remove('active'); });
    document.getElementById(id).classList.add('active');
  }
  function showPortal() { showScreen('portalScreen'); clearPortal(); }
  function showMain()   { showScreen('mainScreen'); }

  function clearPortal() {
    document.getElementById('portalName').value = '';
    document.getElementById('portalMsg').value  = '';
    document.getElementById('charCount').textContent = '0 / 120';
    document.getElementById('charCount').classList.remove('over');
    document.getElementById('portalStatus').textContent = '';
    document.getElementById('portalStatus').className = 'portal-status';
    document.getElementById('portalSendBtn').disabled = false;
  }

  function updateCharCount() {
    var len = document.getElementById('portalMsg').value.length;
    var el  = document.getElementById('charCount');
    el.textContent = len + ' / 120';
    el.classList.toggle('over', len >= 110);
  }

  function sendPortalMsg() {
    var name = document.getElementById('portalName').value.trim();
    var msg  = document.getElementById('portalMsg').value.trim();
    var statusEl = document.getElementById('portalStatus');
    if (!name) {
      statusEl.textContent = 'Please enter your name.';
      statusEl.className = 'portal-status err';
      document.getElementById('portalName').focus();
      return;
    }
    if (!msg) {
      statusEl.textContent = 'Please enter a message.';
      statusEl.className = 'portal-status err';
      document.getElementById('portalMsg').focus();
      return;
    }
    document.getElementById('portalSendBtn').disabled = true;
    statusEl.textContent = 'Sending...';
    statusEl.className = 'portal-status';

    fetch('/api/sendMessage?name=' + encodeURIComponent(name) + '&message=' + encodeURIComponent(msg))
      .then(function(r) { return r.json(); })
      .then(function(d) {
        statusEl.textContent = '✓ Message sent! Ethan will see it soon.';
        statusEl.className = 'portal-status ok';
        document.getElementById('portalMsg').value = '';
        document.getElementById('portalName').value = '';
        updateCharCount();
      })
      .catch(function() {
        statusEl.textContent = 'Failed to send. Check your connection.';
        statusEl.className = 'portal-status err';
        document.getElementById('portalSendBtn').disabled = false;
      });
  }

  var clockColor = localStorage.getItem("clockColor") || "ff0000";
  (function() {
    var map = { "ff0000":"clr-red", "0080ff":"clr-blue", "ffffff":"clr-white" };
    if (map[clockColor]) document.getElementById(map[clockColor]).classList.add("active");
  })();

  function setClockColor(hex) {
    clockColor = hex;
    localStorage.setItem("clockColor", hex);
    var map = { "ff0000":"clr-red", "0080ff":"clr-blue", "ffffff":"clr-white" };
    Object.keys(map).forEach(function(k) {
      document.getElementById(map[k]).classList.toggle("active", k === hex);
    });
  }

  var sleepEnabled  = localStorage.getItem("sleepEnabled") === "true";
  var sleepTime     = localStorage.getItem("sleepTime")    || "23:00";
  var wakeTime      = localStorage.getItem("wakeTime")     || "07:00";
  var loading       = null;
  var boardIsOff    = false;
  var sleepTimeouts = [];

  var inboxMessages  = [];  
  var viewedSet      = {};  
  var inboxPollTimer = null;

  function pollInbox() {
    fetchInboxCount();
    clearInterval(inboxPollTimer);
    inboxPollTimer = setInterval(fetchInboxCount, 15000);
  }

  function fetchInboxCount() {
    fetch('/api/messages')
      .then(function(r) { return r.json(); })
      .then(function(d) {
        inboxMessages = d.messages || [];
        var unread = inboxMessages.filter(function(m) { return !viewedSet[m.id]; }).length;
        updateInboxBadge(unread);
      })
      .catch(function() {});
  }

  function updateInboxBadge(count) {
    var btn   = document.getElementById('inboxBtn');
    var badge = document.getElementById('inboxBadge');
    badge.textContent = count;
    btn.classList.toggle('has-mail', count > 0);
  }

  function openInbox() {
    fetchInboxCount(); 
    renderInboxList();
    document.getElementById('inboxOverlay').classList.add('open');
  }
  function closeInbox() { document.getElementById('inboxOverlay').classList.remove('open'); }

  function renderInboxList() {
    var list = document.getElementById('inboxList');
    list.innerHTML = '';
    if (inboxMessages.length === 0) {
      list.innerHTML = '<div class="inbox-empty">No messages yet...</div>';
      return;
    }
    
    var sorted = inboxMessages.slice().reverse();
    sorted.forEach(function(m) {
      var viewed = !!viewedSet[m.id];
      var item = document.createElement('div');
      item.className = 'inbox-item' + (viewed ? ' viewed' : '');
      item.innerHTML =
        '<div class="inbox-item-left">' +
          '<div class="inbox-item-name">' + escHtml(m.name) + '</div>' +
          '<div class="inbox-item-preview">' + (viewed ? 'Already viewed' : 'Tap to display on board') + '</div>' +
        '</div>' +
        '<span class="inbox-item-time">' + escHtml(m.time) + '</span>' +
        '<span class="inbox-item-arrow">&#9654;</span>';
      item.addEventListener('click', function() { displayInboxMessage(m); });
      list.appendChild(item);
    });
  }

  function displayInboxMessage(m) {
    viewedSet[m.id] = true;
    fetch('/api/displayMessage?id=' + m.id)
      .then(function(r) { return r.json(); })
      .then(function(d) {
        addLog('MSG from ' + m.name + ' → displayed', 'ok');
        var unread = inboxMessages.filter(function(x) { return !viewedSet[x.id]; }).length;
        updateInboxBadge(unread);
        renderInboxList();
      })
      .catch(function() {
        addLog('MSG display failed', 'err');
      });
    closeInbox();
  }

  function addLog(text, type) {
    type = type || "ok";
    var box = document.getElementById("logBox");
    var empty = box.querySelector(".log-empty");
    if (empty) empty.closest(".log-entry").remove();
    var now = new Date().toTimeString().slice(0,8);
    var entry = document.createElement("div");
    entry.className = "log-entry";
    entry.innerHTML = '<span class="log-time">'+now+'</span><span class="log-'+type+'">'+escHtml(text)+'</span>';
    box.insertBefore(entry, box.firstChild);
    while (box.children.length > 20) box.removeChild(box.lastChild);
  }

  function escHtml(s) {
    return String(s).replace(/&/g,"&amp;").replace(/</g,"&lt;").replace(/>/g,"&gt;");
  }

  function setStatus(state) {
    document.getElementById("statusDot").className = "status-dot" + (state ? " "+state : "");
  }

  function setLoading(id) {
    loading = id;
    var ids      = { A:"btnA", B:"btnB", DF:"btnDF", CLK:"btnCLK" };
    var labels   = { A:"0x01", B:"0x02", DF:"AUTO",  CLK:"0x04",   SPT:"0x05"  };
    var spinners = { A:"",     B:"blue", DF:"yellow", CLK:"purple", SPT:"red" };
    Object.keys(ids).forEach(function(k) {
      var btn = document.getElementById(ids[k]);
      var lbl = document.getElementById(ids[k]+"Id");
      btn.disabled = !!id;
      btn.classList.remove("loading");
      lbl.textContent = labels[k];
    });
    document.getElementById("sendBtn").disabled  = !!id;
    document.getElementById("powerBtn").disabled = !!id;
    if (id && ids[id]) {
      setStatus("busy");
      document.getElementById(ids[id]).classList.add("loading");
      document.getElementById(ids[id]+"Id").innerHTML = '<span class="spinner '+spinners[id]+'"></span>RUNNING';
    } else if (!id) {
      setStatus(boardIsOff ? "off" : "");
    }
  }

  async function request(label, path, loadingId) {
    setLoading(loadingId);
    try {
      var res  = await fetch(path);
      var data = await res.json();
      addLog(label + " -> " + data.status, "ok");
      return true;
    } catch(e) {
      addLog(label + " -> connection failed", "err");
      setStatus("error");
      setTimeout(function(){ setStatus(boardIsOff ? "off" : ""); }, 2000);
      return false;
    } finally {
      setLoading(null);
    }
  }

  function runA()       { boardIsOff=false; request("WEATHER",      "/api/programA",       "A");   }
  function runB()       { boardIsOff=false; request("FLIGHT DATA",  "/api/programB",       "B");   }
  function runDefault() { boardIsOff=false; request("DEFAULT MODE", "/api/programDefault", "DF");  }
  function runClock()   { boardIsOff=false; request("CLOCK", "/api/clock?color="+clockColor, "CLK"); }

  async function boardOff() {
    if (loading) return;
    var ok = await request("BOARD OFF", "/api/boardOff", "A");
    if (ok) { boardIsOff=true; setStatus("off"); }
  }

  function sendMsg() {
    var input = document.getElementById("msgInput");
    var msg = input.value.trim();
    if (!msg || loading) return;
    input.value = "";
    boardIsOff = false;
    request('MSG: "'+msg+'"', "/api/display?message="+encodeURIComponent(msg), "A");
  }

  function scheduleSleep() {
    sleepTimeouts.forEach(function(t){ clearTimeout(t); });
    sleepTimeouts = [];
    if (!sleepEnabled) return;
    function msUntil(ts) {
      var p=ts.split(":"), h=parseInt(p[0]), m=parseInt(p[1]);
      var now=new Date(), t=new Date(now);
      t.setHours(h,m,0,0);
      if(t<=now) t.setDate(t.getDate()+1);
      return t-now;
    }
    function fmt(ts) {
      var p=ts.split(":"), h=parseInt(p[0]);
      return ((h%12)||12)+":"+p[1]+(h>=12?" PM":" AM");
    }
    sleepTimeouts.push(setTimeout(function(){
      addLog("Auto sleep ("+fmt(sleepTime)+") -> board off","warn");
      fetch("/api/boardOff").catch(function(){});
      boardIsOff=true; setStatus("off"); scheduleSleep();
    }, msUntil(sleepTime)));
    sleepTimeouts.push(setTimeout(function(){
      addLog("Auto wake ("+fmt(wakeTime)+") -> default mode","info");
      fetch("/api/programDefault").catch(function(){});
      boardIsOff=false; setStatus(""); scheduleSleep();
    }, msUntil(wakeTime)));
    addLog("Schedule: sleep "+fmt(sleepTime)+", wake "+fmt(wakeTime),"info");
  }

  if (sleepEnabled) scheduleSleep();

  var sportsPriority = [0, 1, 2, 3, 4, 5];
  var sportNames = ["NFL (Seahawks)", "MLB (Mariners)", "NHL (Kraken)", "MLS (Sounders)", "NCAAF (Alabama)", "NCAAB (Alabama)"];

  function renderPriorityList() {
    var list = document.getElementById('prioList');
    list.innerHTML = '';
    sportsPriority.forEach(function(sportId, index) {
      var html = '<div class="prio-item">' +
                   '<span>' + (index + 1) + '. ' + sportNames[sportId] + '</span>' +
                   '<div class="prio-controls">' +
                     '<button class="prio-btn" onclick="movePrio(' + index + ', -1)" ' + (index === 0 ? 'disabled' : '') + '>&#8593;</button>' +
                     '<button class="prio-btn" onclick="movePrio(' + index + ', 1)" ' + (index === 5 ? 'disabled' : '') + '>&#8595;</button>' +
                   '</div>' +
                 '</div>';
      list.innerHTML += html;
    });
  }

  function movePrio(index, direction) {
    var newIndex = index + direction;
    if (newIndex < 0 || newIndex > 5) return;
    var temp = sportsPriority[index];
    sportsPriority[index] = sportsPriority[newIndex];
    sportsPriority[newIndex] = temp;
    renderPriorityList();
  }

  function openSettings() {
    document.getElementById("sleepEnabled").checked = sleepEnabled;
    document.getElementById("sleepTime").value      = sleepTime;
    document.getElementById("wakeTime").value       = wakeTime;
    renderPriorityList();
    document.getElementById("settingsOverlay").classList.add("open");
  }
  
  function closeSettings() { document.getElementById("settingsOverlay").classList.remove("open"); }

  function saveSettings() {
    sleepEnabled = document.getElementById("sleepEnabled").checked;
    sleepTime    = document.getElementById("sleepTime").value;
    wakeTime     = document.getElementById("wakeTime").value;
    
    localStorage.setItem("sleepEnabled", sleepEnabled);
    localStorage.setItem("sleepTime",    sleepTime);
    localStorage.setItem("wakeTime",     wakeTime);
    
    var sh=parseInt(sleepTime.split(":")[0]), wh=parseInt(wakeTime.split(":")[0]);
    
    fetch("/api/setSleep?sleepH="+sh+"&wakeH="+wh+"&enabled="+(sleepEnabled?1:0))
      .then(function(r){return r.json();})
      .then(function(d){addLog("Board schedule -> "+d.status,"info");})
      .catch(function(){addLog("Schedule saved locally","warn");});
      
    fetch("/api/setPriority?p=" + sportsPriority.join(','))
      .then(function(r){return r.json();})
      .then(function(d){addLog("Sports priority updated","info");})
      .catch(function(){addLog("Priority saved locally","warn");});
      
    scheduleSleep();
    closeSettings();
  }

  var sportsOpen        = false;
  var sportsAutoRefresh = null;
  var sportsFastPoll    = null;

  function runSports() {
    boardIsOff = false;
    if (!sportsOpen) {
      sportsOpen = true;
      document.getElementById('sportsPanel').classList.add('open');
      document.getElementById('btnSpt').classList.add('active');
      document.getElementById('sportsBody').innerHTML =
        '<div class="sports-empty"><span class="loading-dots">Fetching scores...</span></div>';
      document.getElementById('sportsPanelStatus').textContent = '';
      fetch('/api/programSports').catch(function(){});
      addLog('Sports mode activated — fetching all leagues', 'info');
      startSportsPolling();
    } else {
      sportsOpen = false;
      document.getElementById('sportsPanel').classList.remove('open');
      document.getElementById('btnSpt').classList.remove('active');
      stopSportsPolling();
    }
  }

  function startSportsPolling() {
    stopSportsPolling();
    sportsFastPoll = setInterval(function() {
      loadSportsData(function(ready) {
        if (ready) {
          stopSportsPolling();
          sportsAutoRefresh = setInterval(function(){ loadSportsData(); }, 30000);
        }
      });
    }, 3000);
    loadSportsData(function(ready) {
      if (ready) {
        stopSportsPolling();
        sportsAutoRefresh = setInterval(function(){ loadSportsData(); }, 30000);
      }
    });
  }

  function stopSportsPolling() {
    if (sportsFastPoll)    { clearInterval(sportsFastPoll);    sportsFastPoll    = null; }
    if (sportsAutoRefresh) { clearInterval(sportsAutoRefresh); sportsAutoRefresh = null; }
  }

  function refreshSports() {
    var btn = document.getElementById('sportsRefreshBtn');
    btn.classList.add('spinning');
    document.getElementById('sportsBody').innerHTML =
      '<div class="sports-empty"><span class="loading-dots">Refreshing...</span></div>';
    fetch('/api/programSports').catch(function(){});
    stopSportsPolling();
    setTimeout(function() {
      startSportsPolling();
      loadSportsData(function() { btn.classList.remove('spinning'); });
    }, 600);
  }

  function loadSportsData(cb) {
    fetch('/api/sportsData')
      .then(function(r) { return r.json(); })
      .then(function(data) {
        if (sportsOpen) renderSports(data);
        if (cb) cb(data.ready === true);
      })
      .catch(function() {
        if (sportsOpen) {
          document.getElementById('sportsPanelStatus').textContent = '— CONNECTION ERROR';
          document.getElementById('sportsBody').innerHTML =
            '<div class="sports-empty">Could not reach board.</div>';
        }
        if (cb) cb(false);
      });
  }

  function fmtStatus(s) {
    if (!s) return '';
    var up = s.toUpperCase();
    if (up === 'FIN') return 'FINAL';
    if (up === 'PRE') return 'SCHEDULED';
    return s;
  }

  function statusClass(s) {
    if (!s) return 'status-pre';
    var up = s.toUpperCase();
    if (up === 'FIN') return 'status-fin';
    if (/\d/.test(s) || up === 'LIVE' || up === 'HT' || up === 'HALF') return 'status-live';
    return 'status-pre';
  }

  function renderSports(data) {
    var games   = data.games  || [];
    var fetched = (typeof data.fetched === 'number') ? data.fetched : 0;
    var now     = new Date();
    var timeStr = now.toLocaleTimeString([], {hour:'2-digit', minute:'2-digit'});

    var liveGames  = games.filter(function(g){ return g.found && g.status !== 'FIN'; });
    var finalGames = games.filter(function(g){ return g.found && g.status === 'FIN'; });
    var nextGames  = games.filter(function(g){ return g.nextFound; });
    var lastGames  = games.filter(function(g){ return g.lastFound; });

    nextGames.sort(function(a,b){ return a.nextTime < b.nextTime ? -1 : 1; });

    var statusEl = document.getElementById('sportsPanelStatus');
    if (!data.ready) {
      statusEl.textContent = ' — loading ' + fetched + '/6...';
    } else if (liveGames.length > 0) {
      statusEl.textContent = ' — ' + liveGames.length + ' live · ' + timeStr;
    } else {
      statusEl.textContent = ' — updated ' + timeStr;
    }

    var html = '';

    if (liveGames.length > 0) {
      html += '<div class="sports-section-label">&#128308; Live Now</div>';
      liveGames.forEach(function(g){ html += buildScoreBugHTML(g); });
    }

    if (finalGames.length > 0) {
      html += '<div class="sports-section-label">&#9989; Final</div>';
      finalGames.forEach(function(g){ html += buildScoreBugHTML(g); });
    }

    if (nextGames.length > 0) {
      html += '<div class="sports-section-label">&#128197; Upcoming</div>';
      nextGames.forEach(function(g) {
        html += '<div class="next-game-card">';
        html += '<div class="next-game-top">';
        html +=   '<span class="next-game-sport">' + g.sport.toUpperCase() + '</span>';
        html +=   '<span class="next-game-label">NEXT</span>';
        html += '</div>';
        html += '<div class="next-game-teams">' + g.nextAway +
                ' <span style="color:var(--dimmer)">vs</span> ' + g.nextHome + '</div>';
        html += '<div class="next-game-date">' + g.nextDate + '</div>';
        html += '</div>';
      });
    }

    if (lastGames.length > 0) {
      html += '<div class="sports-section-label">&#128202; Recent Results</div>';
      lastGames.forEach(function(g) {
        var awayWon = g.lastAwayScore > g.lastHomeScore;
        html += '<div class="last-game-card">';
        html += '<div class="last-game-sport">' + g.sport.toUpperCase() + '</div>';
        html += '<div class="last-game-result">';
        html +=   '<span class="' + (awayWon?'w':'') + '">' +
                    g.lastAway + ' ' + g.lastAwayScore + '</span>';
        html += ' &ndash; ';
        html +=   '<span class="' + (!awayWon?'w':'') + '">' +
                    g.lastHome + ' ' + g.lastHomeScore + '</span>';
        html += '</div>';
        html += '</div>';
      });
    }

    if (!html) {
      if (!data.ready) {
        html = '<div class="sports-empty"><span class="loading-dots">Fetching ' +
               fetched + '/6 leagues...</span></div>';
      } else {
        html = '<div class="sports-empty">No game data found.<br>' +
               '<span style="color:var(--dimmer)">SEA: NFL·MLB·NHL·MLS &nbsp; ALA: NCAAF·NCAAB</span></div>';
      }
    }

    document.getElementById('sportsBody').innerHTML = html;
  }

  function buildScoreBugHTML(g) {
    var awayWon = g.status === 'FIN' && g.awayScore > g.homeScore;
    var homeWon = g.status === 'FIN' && g.homeScore > g.awayScore;
    var html = '';
    html += '<div class="score-bug">';
    html += '<div class="score-bug-header">';
    html +=   '<span class="score-bug-sport">' + g.sport.toUpperCase() + '</span>';
    html +=   '<span class="score-bug-status ' + statusClass(g.status) + '">' +
                fmtStatus(g.status) + '</span>';
    html += '</div>';
    html += '<div class="score-bug-row">';
    html +=   '<div class="score-bug-accent accent-away"></div>';
    html +=   '<div class="score-bug-team' + (awayWon?' winner':'') + '">' + g.away + '</div>';
    html +=   '<div class="score-bug-score' + (homeWon?' loser':'') + '">' + g.awayScore + '</div>';
    html += '</div>';
    html += '<div class="score-bug-row">';
    html +=   '<div class="score-bug-accent accent-home"></div>';
    html +=   '<div class="score-bug-team' + (homeWon?' winner':'') + '">' + g.home + '</div>';
    html +=   '<div class="score-bug-score' + (awayWon?' loser':'') + '">' + g.homeScore + '</div>';
    html += '</div>';
    html += '</div>';
    return html;
  }

  // Initialize inbox polling immediately since we skip the login screen
  window.addEventListener('load', function() {
    pollInbox();
  });

</script>
</body>
</html>
)=====";