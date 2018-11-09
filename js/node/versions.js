function fetchVersions(cb) {
  require('child_process').exec('npm ls --depth=0 --json', function(err, stdout, stderr) {
    cb(null, JSON.parse(stdout));
  });
}
fetchVersions(console.log);