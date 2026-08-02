// MIT License — Copyright (c) 2025 Llama UI Native
// See LICENSE file in the project root.
/* i18n.js — Minimal DOM translation for strings not handled by SvelteKit's $state */
(function() {
	var L = (localStorage.getItem('lang') || 'en').trim();
	var D = {}, W = null, R = false;
	var SKIP = new Set(['SCRIPT','STYLE','CODE','PRE','KBD','VAR','SAMP','IFRAME','SVG','MATH','TEMPLATE','NOSCRIPT']);

	function bl() { W = {}; for (var k in D) if (D.hasOwnProperty(k)) W[k.toLowerCase()] = k; }
	function tf(t) { if (D[t]) return D[t]; if (!W) bl(); var k = W[t.toLowerCase()]; return k ? D[k] : null; }
	function ik(r) { var t = r.trim(); if (!t || /^Default:/.test(t)) return null; var m = /^\[([A-Z]{2})\]\s/.exec(t); return m ? t.slice(m[0].length) : t; }

	function tn(node) {
		var p = node.parentElement; if (!p || SKIP.has(p.tagName)) return;
		var k = ik(node.textContent); if (!k) return;
		var t = tf(k); if (!t || t === k) return;
		var raw = node.textContent, idx = raw.indexOf(k);
		if (idx !== -1) node.textContent = raw.slice(0, idx) + t + raw.slice(idx + k.length);
	}

	function walk(root) {
		if (!root || L === 'en') return;
		if (root.nodeType === 3) { var p = root.parentElement; if (p && !SKIP.has(p.tagName)) tn(root); return; }
		if (root.nodeType !== 1) return;
		if (SKIP.has(root.tagName)) return;
		for (var c = root.firstChild; c; c = c.nextSibling) walk(c);
	}

	function scan() { if (R && document.body) walk(document.body); }

	new MutationObserver(function(ms) {
		for (var i = 0; i < ms.length; i++) {
			if (ms[i].type === 'characterData' && R && L !== 'en') tn(ms[i].target);
			if (ms[i].type === 'childList' && R && L !== 'en')
				for (var j = 0; j < ms[i].addedNodes.length; j++) walk(ms[i].addedNodes[j]);
		}
	}).observe(document.documentElement, { childList:true, subtree:true, characterData:true });

	function load(code) {
		R = false; W = null; D = {};
		if (code === 'en') { R = true; document.documentElement.lang = 'en'; return; }
		fetch('/lang/' + code + '.json').then(function(r) { return r.json(); }).then(function(d) {
			D = d; bl(); R = true; document.documentElement.lang = code;
			scan(); [200,500,1500,4000,8000].forEach(function(dly) { setTimeout(scan, dly); });
		}).catch(function(e) { R = true; });
	}

	setInterval(function() { var c = (localStorage.getItem('lang')||'en').trim(); if (c !== L) { L = c; load(c); } }, 300);
	load(L);
})();
