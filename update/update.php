<?php
    class Version {
        const STABLE_PATTERN = '/^(\\d+)\\.(\\d+)\\.(\\d+)()\\.bin$/';
        const UNSTABLE_PATTERN = '/^(\\d+)\\.(\\d+)\\.(\\d+)-(\\d+)\\.bin$/';

        var string $filename;
        var int $major = 0;
        var int $minor = 0;
        var int $patch = 0;
        var int $devel = 0;

        function __construct(String $filename) {
            $this->filename = $filename;

            if (preg_match(Version::STABLE_PATTERN, $this->filename, $matches) ||
                preg_match(Version::UNSTABLE_PATTERN, $this->filename, $matches)) {
                $this->major = intval($matches[1]);
                $this->minor = intval($matches[2]);
                $this->patch = intval($matches[3]);
                $this->devel = intval($matches[4]);
            }
        }

        function isStable() {
            return $this->devel == 0;
        }

        function isUnstable() {
            return !$this->isStable();
        }

        function getBytes() {
            return pack('C*', $this->major, $this->minor, $this->patch, $this->devel);
        }

        function getUrl() {
            $protocol = empty($_SERVER['HTTPS']) ? 'http' : 'https';
            $host = $_SERVER['HTTP_HOST'];
            $path = dirname($_SERVER['PHP_SELF']);

            return "${protocol}://${host}${path}/" . $this->filename;
        }

        function exists() {
            return $this->major || $this->minor || $this->patch;
        }

        function isUnstableVersionOf(Version $stable_version) {
            return
                $this->isUnstable() &&
                $stable_version->isStable() &&
                $this->major == $stable_version->major &&
                $this->minor == $stable_version->minor &&
                $this->patch == $stable_version->patch;
        }

        function compareTo(Version $other) {
            if ($this->major != $other->major) {
                return $this->major > $other->major ? 1 : -1;
            }
            if ($this->minor != $other->minor) {
                return $this->minor > $other->minor ? 1 : -1;
            }
            if ($this->patch != $other->patch) {
                return $this->patch > $other->patch ? 1 : -1;
            }
            if ($this->devel != $other->devel) {
                return $this->devel > $other->devel ? 1 : -1;
            }
            return 0;
        }

        public static function compare($a, $b) {
            return $a->compareTo($b);
        }
    }

    class Versions {
        var $stable = [];
        var $unstable = [];

        function __construct() {
            $filenames = glob('*.bin');

            array_push($this->stable, new Version('0.0.0.bin'));
            array_push($this->unstable, new Version('0.0.0-0.bin'));

            foreach ($filenames as $filename) {
                $version = new Version($filename);
                $version->isStable()
                    ? array_push($this->stable, $version)
                    : array_push($this->unstable, $version);
            }

            usort($this->stable, array("Version", "compare"));
            $this->stable = array_reverse($this->stable);

            usort($this->unstable, array("Version", "compare"));
            $this->unstable = array_reverse($this->unstable);
        }

        private function getNoUnstableVersion() {
            return $this->unstable[count($this->unstable)-1];
        }

        public function getVersion($type, $stability) {
            if ($stability == "stable") {
                if ($type == "current") {
                    return $this->stable[0];
                }
                else if ($type == "previous") {
                    return (count($this->stable) > 1)
                        ? $this->stable[1]
                        : $this->stable[0];
                }
            }
            else if ($stability == "unstable") {
                if ($type == "current") {
                    $unstable_version = $this->unstable[0];
                }
                else if ($type == "previous") {
                    $unstable_version = (count($this->unstable) > 1)
                        ? $this->unstable[1]
                        : $this->unstable[0];
                }

                $current_stable_version = $this->getVersion("current", "stable");

                if ($current_stable_version->exists() &&
                    $unstable_version->isUnstableVersionOf($current_stable_version)) {
                    return $unstable_version;
                }
                else {
                    return $this->getNoUnstableVersion();
                }
            }
        }

        function getVersionAsBytes($type, $stability) {
            return $this->getVersion($type, $stability)->getBytes();
        }

        function getUrl($type, $stability) {
            return $this->getVersion($type, $stability)->getUrl();
        }
    }

    $versions = new Versions();

    if (!isset($_SERVER['HTTP_HOST'])) {
        parse_str($argv[1], $_GET);
    }

    $query = $_GET['q'];

    if (!preg_match('/^(c|p)(s|u)(v|u)$/', $query)) {
        http_response_code(500);
        exit;
    }

    $type = ($query[0] == 'c') ? "current" : "previous";
    $stability = ($query[1] == 's') ? "stable" : "unstable";
    $request = ($query[2] == 'v') ? "version" : "url";

    if ($request == "version") {
        $version = $versions->getVersionAsBytes($type, $stability);
        header("Content-Type: application/octet-stream", true);
        header("Content-Length: 4", true);
        print $version;
    }
    else if ($request == "url") {
        $url = $versions->getUrl($type, $stability);
        header("Content-Type: text/text", true);
        header("Content-Length: " . strlen($url), true);
        print $url;
    }
?>