/**
 * icons.js — Agent B: inline lucide SVG icons (paths from lucide-static,
 * MIT). Keeps the settings/chat UI dependency-free while matching the old
 * app's iconography 1:1.
 */
const PATHS = {
  settings: 'M9.671 4.136a2.34 2.34 0 0 1 4.659 0 2.34 2.34 0 0 0 3.319 1.915 2.34 2.34 0 0 1 2.33 4.033 2.34 2.34 0 0 0 0 3.831 2.34 2.34 0 0 1-2.33 4.033 2.34 2.34 0 0 0-3.319 1.915 2.34 2.34 0 0 1-4.659 0 2.34 2.34 0 0 0-3.319-1.915 2.34 2.34 0 0 1-2.33-4.033 2.34 2.34 0 0 0 0-3.831 2.34 2.34 0 0 1 2.33-4.033 2.34 2.34 0 0 0 3.319-1.915z',
  sliders: ['M10 8h4', 'M12 21v-9', 'M12 8V3', 'M17 16h4', 'M19 12V3', 'M19 21v-5', 'M3 14h4', 'M5 10V3', 'M5 21v-7'].join('|'),
  monitor: 'rect',
  funnel: 'M10 20a1 1 0 0 0 .553.895l2 1A1 1 0 0 0 14 21v-7a2 2 0 0 1 .517-1.341L21.74 4.67A1 1 0 0 0 21 3H3a1 1 0 0 0-.742 1.67l7.225 7.989A2 2 0 0 1 10 14z',
  code: ['m16 18 6-6-6-6', 'm8 6-6 6 6 6'].join('|'),
  'list-restart': ['M21 5H3', 'M7 12H3', 'M7 19H3', 'M12 18a5 5 0 0 0 9-3 4.5 4.5 0 0 0-4.5-4.5c-1.33 0-2.54.54-3.41 1.41L11 14', 'M11 10v4h4'].join('|'),
  'pencil-ruler': [
    'M13 7 8.7 2.7a2.41 2.41 0 0 0-3.4 0L2.7 5.3a2.41 2.41 0 0 0 0 3.4L7 13',
    'm8 6 2-2', 'm18 16 2-2', 'm17 11 4.3 4.3c.94.94.94 2.46 0 3.4l-2.6 2.6c-.94.94-2.46.94-3.4 0L11 14',
    'M21.174 6.812a1 1 0 0 0-3.986-3.987L3.842 16.174a2 2 0 0 0-.5.83l-1.321 4.352a.5.5 0 0 0 .623.622l4.353-1.32a2 2 0 0 0 .83-.497z',
    'm15 5 4 4'
  ].join('|'),
  database: ['M3 5V19A9 3 0 0 0 21 19V5', 'M3 12A9 3 0 0 0 21 12'].join('|'),
  bot: ['M12 8V4H8', 'M2 14h2', 'M20 14h2', 'M15 13v2', 'M9 13v2', 'M9 17h6', 'M19 15V6.5A1.5 1.5 0 0 0 17.5 5H14'].join('|'),
  download: ['M12 15V3', 'M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4', 'm7 10 5 5 5-5'].join('|'),
  upload: ['M12 3v12', 'm17 8-5-5-5 5', 'M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4'].join('|'),
  'refresh-ccw': ['M21 12a9 9 0 0 0-9-9 9.75 9.75 0 0 0-6.74 2.74L3 8', 'M3 3v5h5', 'M3 12a9 9 0 0 0 9 9 9.75 9.75 0 0 0 6.74-2.74L21 16', 'M16 16h5v5'].join('|'),
  search: ['m21 21-4.34-4.34', 'M11 4a7 7 0 1 0 0 14 7 7 0 0 0 0-14'].join('|'),
  'message-square': 'M22 17a2 2 0 0 1-2 2H6.828a2 2 0 0 0-1.414.586l-2.202 2.202A.71.71 0 0 1 2 21.286V5a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2z',
  'panel-left-open': ['M9 3v18', 'm14 9 3 3-3 3'].join('|'),
  'panel-left-close': ['M9 3v18', 'm16 15-3-3 3-3'].join('|'),
  plus: ['M5 12h14', 'M12 5v14'].join('|'),
  'trash-2': ['M10 11v6', 'M14 11v6', 'M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6', 'M3 6h18', 'M8 6V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2'].join('|'),
  star: 'M11.525 2.295a.53.53 0 0 1 .95 0l2.31 4.679a2.123 2.123 0 0 0 1.595 1.16l5.166.756a.53.53 0 0 1 .294.904l-3.736 3.638a2.123 2.123 0 0 0-.611 1.878l.882 5.14a.53.53 0 0 1-.771.56l-4.618-2.428a2.123 2.123 0 0 0-1.976 0l-4.618 2.428a.53.53 0 0 1-.771-.56l.882-5.14a2.123 2.123 0 0 0-.611-1.878L2.414 9.794a.53.53 0 0 1 .294-.904l5.166-.756a2.123 2.123 0 0 0 1.595-1.16z',
  pencil: ['M21.174 6.812a1 1 0 0 0-3.986-3.987L3.842 16.174a2 2 0 0 0-.5.83l-1.321 4.352a.5.5 0 0 0 .623.622l4.353-1.32a2 2 0 0 0 .83-.497z', 'm15 5 4 4'].join('|'),
  sparkles: 'M11.017 2.814a1 1 0 0 1 1.966 0l1.051 5.558a2 2 0 0 0 1.594 1.594l5.558 1.051a1 1 0 0 1 0 1.966l-5.558 1.051a2 2 0 0 0-1.594 1.594l-1.051 5.558a1 1 0 0 1-1.966 0l-1.051-5.558a2 2 0 0 0-1.594-1.594l-5.558-1.051a1 1 0 0 1 0-1.966l5.558-1.051a2 2 0 0 0 1.594-1.594z',
  x: ['M18 6 6 18', 'm6 6 12 12'].join('|'),
  check: 'M20 6 9 17l-5-5',
  'chevron-down': 'm6 9 6 6 6-6',
  'chevron-right': 'm9 18 6-6-6-6',
  globe: ['M12 2a14.5 14.5 0 0 0 0 20 14.5 14.5 0 0 0 0-20', 'M2 12h20'].join('|')
};

const RECTS = {
  monitor: { x: 3, y: 3, w: 18, h: 14, rx: 2, extra: ['M12 17v4', 'M8 21h8'] }
};

/** icon(name, className) -> inline SVG element (stroke-current, lucide style). */
export function icon(name, className = 'h-4 w-4') {
  const ns = 'http://www.w3.org/2000/svg';
  const svg = document.createElementNS(ns, 'svg');
  svg.setAttribute('viewBox', '0 0 24 24');
  svg.setAttribute('fill', 'none');
  svg.setAttribute('stroke', 'currentColor');
  svg.setAttribute('stroke-width', '2');
  svg.setAttribute('stroke-linecap', 'round');
  svg.setAttribute('stroke-linejoin', 'round');
  svg.setAttribute('class', className);
  const def = PATHS[name] ?? '';
  if (def === 'rect') {
    const r = RECTS[name];
    const rect = document.createElementNS(ns, 'rect');
    rect.setAttribute('x', String(r.x));
    rect.setAttribute('y', String(r.y));
    rect.setAttribute('width', String(r.w));
    rect.setAttribute('height', String(r.h));
    rect.setAttribute('rx', String(r.rx));
    svg.appendChild(rect);
    for (const d of r.extra) {
      const p = document.createElementNS(ns, 'path');
      p.setAttribute('d', d);
      svg.appendChild(p);
    }
  } else {
    for (const d of def.split('|')) {
      const p = document.createElementNS(ns, 'path');
      p.setAttribute('d', d);
      svg.appendChild(p);
    }
  }
  return svg;
}
