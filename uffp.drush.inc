<?php

/**
 * @file
 *
 * download Drupal managed files from the production server
 *
 * add this to your ~/.drush directory for it to work
 */

/**
 * Implements hook_drush_command().
 */
function uffp_drush_command() {
  $items = array();

  $items['update-files-from-prod'] = array(
    'description' => 'Download any missing images (or specified file type) from the production server',
    'arguments' => array(
      'production server base url'  => 'The base path of the production server:  or http://example.com/drupal. You can also have the variable "stage_file_proxy_origin" set to bypass this requirement, see https://www.drupal.org/project/stage_file_proxy .',
    ),
    'options' => array(
      'type'    => 'Only fetch file of the same type',
      'fid'     => 'Oly fetch the file with the given id',
    ),
    'examples' => array(
      'drush update-images-from-prod http://example.com' => 'fetch all files from the drupal installation located at http://example.com',
      'drush update-images-from-prod http://example.com/drupal' => 'fetch all files from the drupal installation located at http://example.com/drupal',
      'drush update-images-from-prod http://example.com/drupal --type="flv"' => 'fetch all files from the drupal installation located at http://example.com/drupal which have the extension of flv',
      'drush update-images-from-prod http://example.com/drupal --fid=238' => 'fetch only the file with the ID 238 from the drupal installation located at http://example.com/drupal',
    ),
    'aliases' => array('uffp'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL, // No bootstrap needed
  );

  return $items;
}

function drush_uffp_update_files_from_prod($prod_base = '') {
  $stage_file_proxy_origin = variable_get('stage_file_proxy_origin', FALSE);
  if (empty($prod_base)) {
    if (!$stage_file_proxy_origin) {
      drush_print(dt('A base URL is required', array()));
      return;
    }
    $prod_base = $stage_file_proxy_origin;
    drush_print(dt('Using stage_file_proxy_origin setting of "@prod_base".', array('@prod_base' => $prod_base)));
  }

  $fid = drush_get_option('fid');
  if (!empty($fid)) {
    uffp_fetch_file(file_load($fid), 1, $prod_base);
    return;
  }

  $type = drush_get_option('type');
  $query = db_select('file_managed')
    ->fields('file_managed', array())
    ->condition('uri', 'public://%', 'LIKE');
  if ($type) {
    // add the file extension
    $query->condition('uri', '%.' . $type, 'LIKE');
  }

  $results = $query->execute();

  $rows = $results->fetchAll();
  $total = count($rows);
  foreach ($rows AS $key => $row) {
    uffp_fetch_file($row, $total, $prod_base);
  }
}

function uffp_fetch_file($file, $total, $prod_base = '') {
  $type = drush_get_option('type');
  if ($type) {
    $extension = array_reverse(explode('.', $file->uri));
    $extension = $extension[0];
    if (strtolower($type) != strtolower($extension)) {
      /*
      drush_log(dt('The file at "!fetch_url" does not match the "!type" type so it will not be fetched.',
        array(
          '!fetch_url' => $row->uri,
          '!type' => $type
        )), 'status');
        */
      return;
    }
  }
  // stole this from @see _drush_print_log()
  if (drush_get_context('DRUSH_NOCOLOR')) {
    $red = "[%s]";
    $yellow = "[%s]";
    $green = "[%s]";
  }
  else {
    $red = "\033[31;40m\033[1m[%s]\033[0m";
    $yellow = "\033[1;33;40m\033[1m[%s]\033[0m";
    $green = "\033[1;32;40m\033[1m[%s]\033[0m";
  }

  static $count = 0;
  $count++;
  $per = intval($count/$total * 10000)/100;
  $percent = sprintf($green, sprintf("%10.2f %% ", $per));

  // create the url
  $url = file_create_url($file->uri);
  $url = str_replace($GLOBALS['base_url'] . '/', '', $url);
  $decoded_url = urldecode($url);

  // check to see if the file exists locally
  if (file_exists($decoded_url)) {
    drush_log(dt('!percent The file at "!url" already exists.', array('!url' => $decoded_url, '!percent' => $percent)), 'ok');
  }
  else if (
    strpos($file->filemime, 'image') !== FALSE ||
    (!empty($type) && $extension == $type)
   ) {
    $fetch_url = $prod_base . '/' . $url;
    $file_path = explode('/', $url);
    array_pop($file_path);
    $directory = implode('/', $file_path);
    drush_log(dt('The file at "!fetch_url" will be fetched.', array('!fetch_url' => $fetch_url)));
    $request = drupal_http_request($fetch_url, array('timeout' => 100));
    if ($request->code == '200') {
      file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
      // we have good data and it needs to be saved
      file_put_contents($decoded_url, $request->data);
      if (file_exists($decoded_url)) {
        drush_log(dt('!percent The file at !fetch_url was fetched.', array('!fetch_url' => $fetch_url, '!percent' => $percent)), 'success');
      }
      else {
        drush_log(dt('!percent There was an error writing the file to !url', array('!url' => $url, '!percent' => $percent)), 'error');
      }
    }
    else {
      drush_log(dt('!percent The file at !fetch_url could not be fetched.', array('!fetch_url' => $fetch_url, '!percent' => $percent)), 'failed');
    }
  }

}
