const logEl = document.getElementById('log');

function log(line) {
  logEl.textContent += line + '\n';
}

document.getElementById('send').addEventListener('click', async () => {
  const msg = 'hello from JS';
  const reply = await window.send(msg);
  log('→ JS: ' + msg);
  log(`← C++: ${reply.text} (#${reply.count})`);
});
