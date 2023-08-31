<?php
    header("Content-Type: application/octet-stream", true);

    $query_pattern = '/^(s|u)(v|u)$/';
    $stable_pattern = '/(\\d+)\\.(\\d+)\\.(\\d+)()\\.bin$/';
    $unstable_pattern = '/(\\d+)\\.(\\d+)\\.(\\d+)-(\\d+)\\.bin$/';

    if (!isset($_SERVER['HTTP_HOST'])) {
        parse_str($argv[1], $_GET);
    }

    $query = $_GET['q'];

    if (!preg_match($query_pattern, $query)) {
        http_response_code(500);
        exit;
    }

    $stable_images = ['0.0.0.bin'];
    $unstable_images = ['0.0.0-0.bin'];

    $entries = glob('*.bin');

    foreach ($entries as $entry) {
        if (preg_match($stable_pattern, $entry)) {
            array_push($stable_images, $entry);
        } else if (preg_match($unstable_pattern, $entry)) {
            array_push($unstable_images, $entry);
        }
    }

    rsort($stable_images);
    rsort($unstable_images);

    if ($query[0] == 's') {
        $image = $stable_images[0];
        $pattern = $stable_pattern;
    } else if ($query[0] == 'u') {
        $image = $unstable_images[0];
        $pattern = $unstable_pattern;
    }

    if ($query[1] == 'v') {
        preg_match($pattern, $image, $matches);
        $major = intval($matches[1]);
        $minor = intval($matches[2]);
        $patch = intval($matches[3]);
        $devel = intval($matches[4]);

        header("Content-Length: 4", true);
        print(pack('C*', $major, $minor, $patch, $devel));
    }
    else if ($query[1] == 'u') {
        $protocol = empty($_SERVER['HTTPS']) ? 'http' : 'https';
        $host = $_SERVER['HTTP_HOST'];
        $path = dirname($_SERVER['PHP_SELF']);

        print "${protocol}://${host}${path}/${image}";
    }
?>