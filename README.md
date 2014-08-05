update_files_from_production
============================

Drush utility to fetch all Drupal managed public files from a production server into the local install

put this repositiory in ~/.drush

usage:
drush uffp http://www.example.com

Drush will then go through the files_managed table in the database and for each image in the "public://" domain it will figure out a url path using http://www.example.com as the base and attempt to fetch the file. If the file already exists on the local file system it will notify the user that the file is already present. On successful download of remote file to local file system or failure the user will be notified.

Generally images are the most desired files types to have on development machines since missing images make the site look more broken than other missing assets. If you wish to download other file types you can specify the file extensionand all files with that extension will be downloaded. This example will download all FLV files from theproduction server:

drush update-images-from-prod http://example.com/drupal --type="flv"
