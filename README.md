update_files_from_production
============================

Drush utility to fetch all Drupal managed files from a production server into the local install

put this repositiory in ~/.drush

usage:
drush uffp http://www.example.com

Drush will then go through the files_managed table in the database and for each file in the "public://" domain it will figure out a url path using http://www.example.com as the base and attempt to fetch the file. If the file already exists on the local file system it will notify the user that the file is already present. On successful download of remote file to local file system or failure the user will be notified.
