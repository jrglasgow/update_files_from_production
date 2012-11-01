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
    'description' => 'Download any missing images from the production server',
    'arguments' => array(
      'production server base url' => 'The base path of the production server:  or http://example.com/drupal',
    ),
    'examples' => array(
      'drush update-images-from-prod http://example.com' => 'fetch all files from the drupal installation located at http://example.com',
      'drush update-images-from-prod http://example.com/drupal' => 'fetch all files from the drupal installation located at http://example.com/drupal',
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

  $sql = "SELECT * FROM {file_managed}";
  $results = db_select('file_managed')->fields('file_managed', array())->execute();
  $rows = $results->fetchAll();

  foreach ($rows AS $key => $row) {
    $url = file_create_url($row->uri);
    $url = str_replace($GLOBALS['base_url'] . '/', '', $url);

    if (!file_exists($url) && strpos($row->filemime, 'image') !== FALSE) {
      $count++;

      $fetch_url = $prod_base . '/' . $url;
      $directory = str_replace($row->filename, '', $url);

      $request = drupal_http_request($fetch_url);
      if ($request->code == '200') {
        file_prepare_directory($directory, FILE_CREATE_DIRECTORY | FILE_MODIFY_PERMISSIONS);
        // we have good data and it needs to be saved
        file_put_contents($url, $request->data);
        if (file_exists($url)) {
          drush_print(dt('The file at !fetch_url was fetched.', array('!fetch_url' => $fetch_url)));
        }
        else {
          drush_print(dt('There was an error writing the files to !url', array('!url' => $url)));
        }
      }
      else {
        drush_print(dt('The file at !fetch_url could not be fetched.', array('!fetch_url' => $fetch_url)));
      }
    }
  }
}
