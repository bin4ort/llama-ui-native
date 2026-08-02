/* i18n.js — Minimal DOM fallback. No polling — bundle handles language switch natively. */
(function() {
	var L = (localStorage.getItem('lang') || 'en').trim();
	var D = {}, W = null;
	var SKIP = new Set(['SCRIPT','STYLE','CODE','PRE','KBD','VAR','SAMP','IFRAME','SVG','MATH','TEMPLATE','NOSCRIPT']);

	function bl() { W = {}; for (var k in D) if (D.hasOwnProperty(k)) W[k.toLowerCase()] = k; }
	function tf(t) { if (D[t]) return D[t]; if (!W) bl(); var k = W[t.toLowerCase()]; return k ? D[k] : null; }
	function ik(r) { var t = r.trim(); if (!t || /^Default:/.test(t)) return null; var m = /^\[([A-Z]{2})\]\s/.exec(t); return m ? t.slice(m[0].length) : t; }

	function tn(node) {
		if (L === 'en') return;
		var p = node.parentElement; if (!p || SKIP.has(p.tagName)) return;
		var k = ik(node.textContent); if (!k) return;
		var t = tf(k); if (!t || t === k) return;
		var raw = node.textContent, idx = raw.indexOf(k);
		if (idx !== -1) node.textContent = raw.slice(0, idx) + t + raw.slice(idx + k.length);
	}

	function walk(root) {
		if (!root || L === 'en') return;
		if (root.nodeType === 3) { tn(root); return; }
		if (root.nodeType !== 1 || SKIP.has(root.tagName)) return;
		for (var c = root.firstChild; c; c = c.nextSibling) walk(c);
	}

	function scan() { if (document.body) walk(document.body); }

	new MutationObserver(function(ms) {
		for (var i = 0; i < ms.length; i++) {
			if (ms[i].type === 'characterData') tn(ms[i].target);
			if (ms[i].type === 'childList')
				for (var j = 0; j < ms[i].addedNodes.length; j++) walk(ms[i].addedNodes[j]);
		}
	}).observe(document.documentElement, { childList:true, subtree:true, characterData:true });

	function load(code) {
		D = {}; W = null;
		if (code === 'en') return;
		fetch('/lang/' + code + '.json').then(function(r) { return r.json(); }).then(function(d) {
			D = d; bl(); scan(); [500,1500,4000].forEach(function(dly) { setTimeout(scan, dly); });
		});
	}

	/* Detect language changes from bundle (same-tab via polling, cross-tab via storage) */
	var _last = L;
	setInterval(function() {
		var c = (localStorage.getItem('lang')||'en').trim();
		if (c !== _last) { _last = L = c; load(c); }
	}, 500);
	window.addEventListener('storage', function(e) {
		if (e.key === 'lang') { L = (e.newValue||'en').trim(); load(L); }
	});

	load(L);
})();
