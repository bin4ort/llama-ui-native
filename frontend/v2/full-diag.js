/* full-diag.js — Clicks through all UI sections, collects untranslated text */
(function() {
  var ORIG = localStorage.getItem('lang') || 'en';
  var _results = {};
  var _dict = null;

  function collectDOM(dict) {
    _dict = dict;
    var w = document.createTreeWalker(document.body, 4);
    var n;
    while ((n = w.nextNode())) {
      var p = n.parentElement;
      if (!p || /SCRIPT|STYLE|CODE|PRE/.test(p.tagName)) continue;
      var t = n.textContent.trim();
      if (t.length < 2) continue;
      if (dict[t] && dict[t] !== t && !t.startsWith('{')) {
        _results[t] = (_results[t] || 0) + 1;
      }
    }
  }

  function clickTab(text, wait, callback) {
    var links = document.querySelectorAll('a, button');
    for (var i = 0; i < links.length; i++) {
      if (links[i].textContent.trim().indexOf(text) === 0) {
        links[i].click();
        setTimeout(callback, wait || 1500);
        return;
      }
    }
    setTimeout(callback, 200);
  }

  function scan(code, done) {
    localStorage.setItem('lang', code);
    setTimeout(function() {
      fetch('/lang/' + code + '.json').then(r => r.json()).then(function(d) {
        _results = {};
        // Click through settings tabs: General, Display, Sampling, Penalties, Tools, Agentic, Developer, Import/Export
        var tabs = ['General', 'Display', 'Sampling', 'Penalties', 'Tools', 'Agentic', 'Developer', 'Import'];
        function next(i) {
          if (i >= tabs.length) {
            // Done — report results
            var keys = Object.keys(_results);
            keys.sort(function(a,b) { return _results[b] - _results[a]; });
            var report = keys.slice(0, 100).map(function(k, i) {
              return (i+1) + '. ' + k.substring(0, 70);
            }).join('\\n');
            console.log('=== ' + code.toUpperCase() + ' UNTRANSLATED (' + keys.length + ') ===');
            console.log(report);
            if (done) done();
            return;
          }
          clickTab(tabs[i], 1500, function() { collectDOM(d); next(i+1); });
        }
        // Navigate to settings first
        clickTab('Settings', 1500, function() { collectDOM(d); next(0); });
      });
    }, 1000);
  }

  scan('de', function() {
    setTimeout(function() {
      localStorage.setItem('lang', ORIG);
      console.log('=== DIAG COMPLETE ===');
    }, 1000);
  });
})();
