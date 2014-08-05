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
      'production server base url'  => 'The base path of the production server:  or http://example.com/drupal',
    ),
    'options' => array(
      'type'    => 'Only fetch file of the same type',
    ),
    'examples' => array(
      'drush update-images-from-prod http://example.com' => 'fetch all files from the drupal installation located at http://example.com',
      'drush update-images-from-prod http://example.com/drupal' => 'fetch all files from the drupal installation located at http://example.com/drupal',
      'drush update-images-from-prod http://example.com/drupal --type="flv"' => 'fetch all files from the drupal installation located at http://example.com/drupal which have the extension of flv',
    ),
    'aliases' => array('uffp'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL, // No bootstrap needed
  );

  return $items;
}

function drush_uffp_update_files_from_prod($prod_base = '') {
  if (empty($prod_base)) {
    drush_print(dt('A base URL is required', array()));
    return;
  }

  $type = drush_get_option('type');

  $results = db_select('file_managed')
    ->fields('file_managed', array())
    ->condition('uri', 'public://%', 'LIKE')
    ->execute();
  $rows = $results->fetchAll();

  foreach ($rows AS $key => $row) {
    if ($type) {
      $extension = array_reverse(explode('.', $row->uri));
      $extension = $extension[0];
      if (strtolower($type) != strtolower($extension)) {
        /*
        drush_log(dt('The file at "!fetch_url" does not match the "!type" type so it will not be fetched.',
          array(
            '!fetch_url' => $row->uri,
            '!type' => $type
          )), 'status');
          */
        continue;
      }
    }
    $url = file_create_url($row->uri);
    $url = str_replace($GLOBALS['base_url'] . '/', '', $url);
    if (file_exists($url)) {
      drush_log(dt('The file at !url already exists.', array('!url' => $url)), 'ok');
    }
    else if (
              strpos($row->filemime, 'image') !== FALSE ||
              (!empty($type) && $extension == $type)
             ) {
      $count++;

      $fetch_url = $prod_base . '/' . $url;
      $file_path = explode('/', $url);
      array_pop($file_path);
      $directory = implode('/', $file_path);
      drush_log(dt('The file at "!fetch_url" will be fetched.', array('!fetch_url' => $fetch_url)));
      $request = drupal_http_request($fetch_url);
      if ($request->code == '200') {
        file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
        // we have good data and it needs to be saved
        file_put_contents($url, $request->data);
        if (file_exists($url)) {
          drush_log(dt('The file at !fetch_url was fetched.', array('!fetch_url' => $fetch_url)), 'success');
        }
        else {
          drush_log(dt('There was an error writing the files to !url', array('!url' => $url)), 'error');
        }
      }
      else {
        drush_log(dt('The file at !fetch_url could not be fetched.', array('!fetch_url' => $fetch_url)), 'error');
      }
    }
  }
}
