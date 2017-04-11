<?php
/**
 * @file
 * Functions for purging Islandora object via drush.
 */

/**
 * Implements hook_drush_command().
 */
function ubl_batch_purge_items_drush_command() {
  $items['purge_items'] = array(
    'description' => 'Purge all items in the ids_file. WARNING: this cannot be undone! Think 3 times before using this command!',
    'options' => array(
      'ids_file' => 'Mandatory, the absolute filepath to a file containing a list of Islandora identifiers to be purged.',
    ),
    'aliases' => array(),
    'examples' => array(
      'drush --user=admin purge_items --ids_file=/path/to/idsfile.csv',
    ),
  );
  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_ubl_batch_purge_items_purge_items_validate() {
  $idsfile = drush_get_option('ids_file');

  // Check ids_file option.
  if (isset($idsfile)) {
    if (!file_exists($idsfile)) {
      return drush_set_error("ids_file '$idsfile' does not exist");
    }
  }

  return TRUE;
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_ubl_batch_purge_items_purge_items() {
  $idsfile = drush_get_option('ids_file');

  if (isset($idsfile)) {
    $ids = islandora_ids_to_purge_from_file($idsfile);

    if (isset($ids[0]) && !islandora_object_load($ids[0])) {
      // first item is not a islandora object, but the header;p remove it from the list.
      array_shift($ids);
    }

    $confirmcommand = 'Purge '.count($ids).' items';
    $value = drush_prompt(dt("You are about to purge @count items. Are you very sure you want to do that? If so, type \"@cmd\" to confirm.", array ('@count' => count($ids), '@cmd' => $confirmcommand)));
    if ($value !== $confirmcommand) {
      return drush_user_abort("Wrong confirmation. Aborted!");
    }
    foreach ($ids as $itemid) {
      $obj = islandora_object_load($itemid);
      if ($obj) {
        $success = islandora_delete_object($obj);
        if ($success) {
          drush_log("Purged item $itemid", "ok");
        }
        else {
          drush_log("Failed to purge item $itemid", "error");
        }
      }
    }
  }
}

function islandora_ids_to_purge_from_file($idsfile) {
  $foundids = array();

  $idsstr = file_get_contents($idsfile);
  $fieldseparator = "";
  $ids = str_getcsv($idsstr, "\n");
  foreach ($ids as $index => $id) {
    if (strlen($id) == 0) {
      continue;
    }
    if ($fieldseparator === "") {
      $fieldseparator = "\t";
      $comps = str_getcsv($id, $fieldseparator);
      if (count($comps) > 1) {
        // Fields are separated by a tab.
      }
      else {
        $fieldseparator = ",";
        $comps = str_getcsv($id, $fieldseparator);
        if (count($comps) > 1) {
          // Fields are separated by a comma.
        }
        else {
          $fieldseparator = ";";
          $comps = str_getcsv($id, $fieldseparator);
          if (count($comps) > 1) {
            // Fields are separated by a semicolon.
          }
          else {
            // Fields are not separated.
            $fieldseparator = FALSE;
          }
        }
      }
    }
    if ($fieldseparator !== FALSE) {
      $comps = str_getcsv($id, $fieldseparator);
      if (count($comps) > 1) {
        $id = $comps[0];
      }
    }
    $foundids[] = $id;
  }
  return $foundids;
}

