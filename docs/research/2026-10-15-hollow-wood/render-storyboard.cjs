// Research-diagram renderer only. Uses the bundled dependency; installs nothing.
const sharp = require('C:/Users/mrbos/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/sharp');
const path = require('node:path');
const root = __dirname;
sharp(path.join(root, '15-value-storyboards.svg'), {density: 100})
  .png().toFile(path.join(root, '15-value-storyboards.png'))
  .then(info => console.log(JSON.stringify(info)))
  .catch(error => { console.error(error.message); process.exitCode = 1; });
