/* The desktop itself is the archived DESK.EXE. This file only hosts DOSBox. */
const screen = document.querySelector('#screen');
const status = document.querySelector('#status');
const start = document.querySelector('#start');
const restart = document.querySelector('#restart');
const fullscreen = document.querySelector('#fullscreen');
let runtime = null;
let desktopStarted = false;

function fitScreen() {
  const wrap = document.querySelector('.screen-wrap');
  const toolbar = document.querySelector('.toolbar');
  const height = Math.max(180, innerHeight - wrap.getBoundingClientRect().top - toolbar.offsetHeight - 75);
  const width = Math.floor(Math.min(1280, wrap.clientWidth, height * 4 / 3));
  screen.style.setProperty('--screen-width', `${width}px`);
  screen.style.setProperty('--screen-height', `${Math.floor(width * 3 / 4)}px`);
}
addEventListener('resize', fitScreen);
fitScreen();
screen.addEventListener('contextmenu', event => event.preventDefault());
new MutationObserver(() => {
  if (screen.width === 640 && screen.height === 480) {
    desktopStarted = true;
    status.textContent = 'Desktop running — right click to quit';
  } else if (desktopStarted) {
    status.textContent = 'Desktop closed — use Restart desktop to return';
  }
  fitScreen();
}).observe(screen, { attributes: true, attributeFilter: ['width', 'height'] });

function unlockAudio() {
  const context = runtime?.dos.SDL?.audioContext;
  if (context && context.state !== 'running') context.resume().catch(console.warn);
}
screen.addEventListener('pointerup', unlockAudio);
screen.addEventListener('keydown', unlockAudio);

start.addEventListener('click', async () => {
  start.disabled = true;
  status.textContent = 'Starting the original desktop…';
  try {
    if (navigator.audioSession) navigator.audioSession.type = 'playback';
  } catch (_) { /* This optional API is not available in every browser. */ }
  try {
    const executable = fetch('original/DESK.EXE').then(response => {
      if (!response.ok) throw new Error(`Could not load DESK.EXE (${response.status})`);
      return response.arrayBuffer();
    });
    const dos = await Dos(screen, {
      wdosboxUrl: 'vendor/js-dos/wdosbox.js',
      cycles: 'fixed 12000',
      autolock: false,
      onprogress: () => {},
      onerror: message => { status.textContent = `Desktop error: ${message}`; },
      log: message => console.info(message),
    });
    dos.fs.createFile('DESK.EXE', await executable);
    runtime = await dos.main(['-c', 'DESK.EXE']);
    document.querySelector('#start-panel').hidden = true;
    restart.disabled = false;
    fullscreen.disabled = false;
    if (desktopStarted) status.textContent = 'Desktop running — right click to quit';
    screen.focus();
    fitScreen();
  } catch (error) {
    console.error(error);
    status.textContent = `Could not start: ${error.message || error}`;
    start.disabled = false;
  }
});

restart.addEventListener('click', () => location.reload());
fullscreen.addEventListener('click', () => {
  document.querySelector('.screen-wrap').requestFullscreen?.();
});
