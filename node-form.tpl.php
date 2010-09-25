<?php
// $Id: node-form.tpl.php,v 1.3.2.1 2010/09/25 19:27:57 voxpelli Exp $

/**
 * @file
 * Template that renders a part of the node form
 */
?>
<div class="<?php print $class; ?>">
  <?php print drupal_render_children($form); ?>
</div>