// i18n.svelte.ts — $state object. Mutate properties, don't reassign.
import { browser } from '$app/environment';

export const tr = $state({
  Settings: 'Settings', General: 'General', Display: 'Display',
  Sampling: 'Sampling', Penalties: 'Penalties', Tools: 'Tools',
  Agentic: 'Agentic', Developer: 'Developer', ImpExp: 'Import / Export',
  Language: 'Language', Theme: 'Theme', SvrEndpoint: 'Server Endpoint',
  WebUiAddr: 'Web UI Address', EnableWebUi: 'Enable Web UI',
  ApiKey: 'API Key', SysMsg: 'System Message', NewChat: 'New Chat',
  TypeMsg: 'Type a message...', Search: 'Search', Reload: 'Reload app',
  Back: 'Back to Chat', Send: 'Send', Reset: 'Reset to default',
  Save: 'Save settings', ShowStats: 'Show message generation statistics',
  Temp: 'Temperature', TopK: 'Top K', TopP: 'Top P', MinP: 'Min P',
  MaxTokens: 'Max tokens', Samplers: 'Samplers',
  dict: {} as Record<string,string>,
});

const DE: Record<string,string> = {
  Settings:"Einstellungen",General:"Allgemein",Display:"Anzeige",
  Sampling:"Sampling",Penalties:"Strafen",Tools:"Werkzeuge",
  Agentic:"Agentisch",Developer:"Entwickler","Import / Export":"Import / Export",
  Language:"Sprache",Theme:"Design","Server Endpoint":"Server-Adresse",
  "Web UI Address":"Web-UI-Adresse","Enable Web UI":"Web-UI aktivieren",
  "API Key":"API-Schlüssel","System Message":"Systemnachricht",
  "New Chat":"+ Neuer Chat","Type a message...":"Nachricht eingeben...",
  Search:"Suche","Reload app":"App neu laden","Back to Chat":"Zurück zum Chat",
  Send:"Senden","Reset to default":"Auf Standard zurücksetzen",
  "Save settings":"Einstellungen speichern",
  "Show message generation statistics":"Nachrichten-Statistiken",
  Temperature:"Temperatur","Top K":"Top K","Top P":"Top P","Min P":"Min P",
  "Max tokens":"Max. Tokens",Samplers:"Sampler",
  English:"Englisch",Deutsch:"Deutsch",
};

const RU: Record<string,string> = {
  Settings:"Настройки",General:"Общие",Display:"Отображение",
  Sampling:"Сэмплирование",Penalties:"Штрафы",Tools:"Инструменты",
  Agentic:"Агентные",Developer:"Разработчик","Import / Export":"Импорт / Экспорт",
  Language:"Язык",Theme:"Тема","Server Endpoint":"Серверный эндпоинт",
  "Web UI Address":"Адрес веб-интерфейса","Enable Web UI":"Включить веб-интерфейс",
  "API Key":"API-ключ","System Message":"Системное сообщение",
  "New Chat":"Новый чат","Type a message...":"Введите сообщение...",
  Search:"Поиск","Reload app":"Перезагрузить приложение","Back to Chat":"Вернуться в чат",
  Send:"Отправить","Reset to default":"Сбросить по умолчанию",
  "Save settings":"Сохранить настройки",
  "Show message generation statistics":"Показывать статистику генерации сообщений",
  Temperature:"Температура","Top K":"Top K","Top P":"Top P","Min P":"Min P",
  "Max tokens":"Макс. токенов",Samplers:"Сэмплеры",
  English:"Английский",Deutsch:"Немецкий","Русский":"Русский",
};

function applyDict(dict: Record<string,string>) {
  for (const k in dict) {
    if (dict.hasOwnProperty(k)) tr.dict[k] = dict[k];
  }
  tr.dict["Русский"] = "Русский";
}

function applyCode(code: string) {
  const isDe = code === 'de';
  const isRu = code === 'ru';

  tr.Settings = isRu ? RU.Settings : isDe ? DE.Settings : 'Settings';
  tr.General = isRu ? RU.General : isDe ? DE.General : 'General';
  tr.Display = isRu ? RU.Display : isDe ? DE.Display : 'Display';
  tr.Sampling = isRu ? RU.Sampling : isDe ? DE.Sampling : 'Sampling';
  tr.Penalties = isRu ? RU.Penalties : isDe ? DE.Penalties : 'Penalties';
  tr.Tools = isRu ? RU.Tools : isDe ? DE.Tools : 'Tools';
  tr.Agentic = isRu ? RU.Agentic : isDe ? DE.Agentic : 'Agentic';
  tr.Developer = isRu ? RU.Developer : isDe ? DE.Developer : 'Developer';
  tr.ImpExp = isRu ? RU["Import / Export"] : isDe ? DE["Import / Export"] : 'Import / Export';
  tr.Language = isRu ? RU.Language : isDe ? DE.Language : 'Language';
  tr.Theme = isRu ? RU.Theme : isDe ? DE.Theme : 'Theme';
  tr.SvrEndpoint = isRu ? RU["Server Endpoint"] : isDe ? DE["Server Endpoint"] : 'Server Endpoint';
  tr.WebUiAddr = isRu ? RU["Web UI Address"] : isDe ? DE["Web UI Address"] : 'Web UI Address';
  tr.EnableWebUi = isRu ? RU["Enable Web UI"] : isDe ? DE["Enable Web UI"] : 'Enable Web UI';
  tr.ApiKey = isRu ? RU["API Key"] : isDe ? DE["API Key"] : 'API Key';
  tr.SysMsg = isRu ? RU["System Message"] : isDe ? DE["System Message"] : 'System Message';
  tr.NewChat = isRu ? RU["New Chat"] : isDe ? DE["New Chat"] : 'New Chat';
  tr.TypeMsg = isRu ? RU["Type a message..."] : isDe ? DE["Type a message..."] : 'Type a message...';
  tr.Search = isRu ? RU.Search : isDe ? DE.Search : 'Search';
  tr.Reload = isRu ? RU["Reload app"] : isDe ? DE["Reload app"] : 'Reload app';
  tr.Back = isRu ? RU["Back to Chat"] : isDe ? DE["Back to Chat"] : 'Back to Chat';
  tr.Send = isRu ? RU.Send : isDe ? DE.Send : 'Send';
  tr.Reset = isRu ? RU["Reset to default"] : isDe ? DE["Reset to default"] : 'Reset to default';
  tr.Save = isRu ? RU["Save settings"] : isDe ? DE["Save settings"] : 'Save settings';
  tr.ShowStats = isRu ? RU["Show message generation statistics"] : isDe ? DE["Show message generation statistics"] : 'Show message generation statistics';
  tr.Temp = isRu ? RU.Temperature : isDe ? DE.Temperature : 'Temperature';
  tr.TopK = isRu ? RU["Top K"] : isDe ? DE["Top K"] : 'Top K';
  tr.TopP = isRu ? RU["Top P"] : isDe ? DE["Top P"] : 'Top P';
  tr.MinP = isRu ? RU["Min P"] : isDe ? DE["Min P"] : 'Min P';
  tr.MaxTokens = isRu ? RU["Max tokens"] : isDe ? DE["Max tokens"] : 'Max tokens';
  tr.Samplers = isRu ? RU.Samplers : isDe ? DE.Samplers : 'Samplers';

  // Dict entries for help text and dynamic strings — loaded from /lang/{code}.json at runtime
  if (browser) {
    fetch('/lang/' + code + '.json')
      .then(r => r.json())
      .then(d => applyDict(d))
      .catch(() => {});
  }

  // Immediate dict entries for static strings
  tr.dict["Import / Export"] = isRu ? RU["Import / Export"] : isDe ? DE["Import / Export"] : 'Import / Export';
  tr.dict["Server Endpoint"] = isRu ? RU["Server Endpoint"] : isDe ? DE["Server Endpoint"] : 'Server Endpoint';
  tr.dict["Web UI Address"] = isRu ? RU["Web UI Address"] : isDe ? DE["Web UI Address"] : 'Web UI Address';
  tr.dict["Enable Web UI"] = isRu ? RU["Enable Web UI"] : isDe ? DE["Enable Web UI"] : 'Enable Web UI';
  tr.dict["API Key"] = isRu ? RU["API Key"] : isDe ? DE["API Key"] : 'API Key';
  tr.dict["System Message"] = isRu ? RU["System Message"] : isDe ? DE["System Message"] : 'System Message';
  tr.dict["New Chat"] = isRu ? RU["New Chat"] : isDe ? DE["New Chat"] : 'New Chat';
  tr.dict["Type a message..."] = isRu ? RU["Type a message..."] : isDe ? DE["Type a message..."] : 'Type a message...';
  tr.dict["Reload app"] = isRu ? RU["Reload app"] : isDe ? DE["Reload app"] : 'Reload app';
  tr.dict["Back to Chat"] = isRu ? RU["Back to Chat"] : isDe ? DE["Back to Chat"] : 'Back to Chat';
  tr.dict["Reset to default"] = isRu ? RU["Reset to default"] : isDe ? DE["Reset to default"] : 'Reset to default';
  tr.dict["Save settings"] = isRu ? RU["Save settings"] : isDe ? DE["Save settings"] : 'Save settings';
  tr.dict["Show message generation statistics"] = isRu ? RU["Show message generation statistics"] : isDe ? DE["Show message generation statistics"] : 'Show message generation statistics';
  tr.dict["Top K"] = isRu ? RU["Top K"] : isDe ? DE["Top K"] : 'Top K';
  tr.dict["Top P"] = isRu ? RU["Top P"] : isDe ? DE["Top P"] : 'Top P';
  tr.dict["Min P"] = isRu ? RU["Min P"] : isDe ? DE["Min P"] : 'Min P';
  tr.dict["Max tokens"] = isRu ? RU["Max tokens"] : isDe ? DE["Max tokens"] : 'Max tokens';
  tr.dict["Settings"] = isRu ? RU.Settings : isDe ? DE.Settings : 'Settings';
  tr.dict["General"] = isRu ? RU.General : isDe ? DE.General : 'General';
  tr.dict["Display"] = isRu ? RU.Display : isDe ? DE.Display : 'Display';
  tr.dict["Sampling"] = isRu ? RU.Sampling : isDe ? DE.Sampling : 'Sampling';
  tr.dict["Penalties"] = isRu ? RU.Penalties : isDe ? DE.Penalties : 'Penalties';
  tr.dict["Tools"] = isRu ? RU.Tools : isDe ? DE.Tools : 'Tools';
  tr.dict["Agentic"] = isRu ? RU.Agentic : isDe ? DE.Agentic : 'Agentic';
  tr.dict["Developer"] = isRu ? RU.Developer : isDe ? DE.Developer : 'Developer';
  tr.dict["Language"] = isRu ? RU.Language : isDe ? DE.Language : 'Language';
  tr.dict["Theme"] = isRu ? RU.Theme : isDe ? DE.Theme : 'Theme';
  tr.dict["Search"] = isRu ? RU.Search : isDe ? DE.Search : 'Search';
  tr.dict["Send"] = isRu ? RU.Send : isDe ? DE.Send : 'Send';
  tr.dict["Temperature"] = isRu ? RU.Temperature : isDe ? DE.Temperature : 'Temperature';
  tr.dict["Samplers"] = isRu ? RU.Samplers : isDe ? DE.Samplers : 'Samplers';
  tr.dict["English"] = isRu ? RU.English : isDe ? DE.English : 'English';
  tr.dict["Deutsch"] = isRu ? RU.Deutsch : isDe ? DE.Deutsch : 'Deutsch';

  tr.dict["Hello there"] = isRu ? "Здравствуйте" : isDe ? "Hallo" : "Hello there";
  tr.dict["Hello there!"] = isRu ? "Здравствуйте!" : isDe ? "Hallo!" : "Hello there!";
  tr.dict["Type a message or upload a file to get started"] = isRu ? "Введите сообщение или загрузите файл" : isDe ? "Nachricht eingeben oder Datei hochladen" : "Type a message or upload a file to get started";
  tr.dict["Search conversations..."] = isRu ? "Поиск по беседам..." : isDe ? "Unterhaltungen suchen..." : "Search conversations...";
}

if (browser) {
  const saved = (localStorage.getItem('lang') || 'en').trim();
  applyCode(saved);

  // Listen for runtime language changes from the settings dropdown
  window.addEventListener('storage', (e) => {
    if (e.key === 'lang' && e.newValue) applyCode(e.newValue.trim());
  });
  // Poll as fallback (storage events don't fire on same tab)
  let _last = saved;
  setInterval(() => {
    const cur = (localStorage.getItem('lang') || 'en').trim();
    if (cur !== _last) { _last = cur; applyCode(cur); }
  }, 500);
}

export function t(key: string): string {
  return tr.dict[key] || key;
}

export function applyLang(code: string) {
  localStorage.setItem('lang', code);
  applyCode(code.trim());
}
