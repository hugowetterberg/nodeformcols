<?php
// $Id$

function _nodeformcols_get_node_type_form($type) {
  module_load_include('inc', 'node', 'node.pages');
  
  $node = array('uid' => $user->uid, 'name' => (isset($user->name) ? $user->name : ''), 'type' => $type, 'language' => '');
  $nfid = $type . '_node_form';
  $fs = array('storage' => NULL, 'submitted' => FALSE);
  $nf = drupal_retrieve_form($nfid, $fs, $node);
  drupal_prepare_form($nfid, $nf, $fs);
  drupal_process_form($nfid, $nf, $fs);
  return $nf;
}

function nodeformcols_update_placements($type, &$placements) {
  $form = _nodeformcols_get_node_type_form($type);
  
  foreach ($form as $key => $field) {
    if (substr($key, 0, 1)=='#' || substr($key, 0, 8)=='section_' || 
        $field['#type']=='value' || $field['#type']=='hidden'|| $field['#type']=='token') {
      continue;
    }
    
    // Ensure a weight so that we don't have to worry about it later
    $field['#weight'] = isset($field['#weight']) ? $field['#weight'] : 0;
    
    if (!isset($placements[$key])) {
      // If the element is unknown we'll add it to 
      // the field placement list
      $placements[$key] = array(
        'region' => 'footer', 
        'weight' => $field['#weight'],
      );
    }
    else if (!isset($placements[$key]['weight'])) {
      $placements[$key]['weight'] = $field['#weight'];
    }
    
    if (isset($field['#collapsible']) && !isset($placements[$key]['collapsed'])) {
      $placements[$key]['collapsed'] = isset($field['#collapsed']) && $field['#collapsed'];
    }
  }
}

function nodeformcols_configuration_form($form_state, $type) {
  $placements = nodeformscols_field_placements($type);  
  nodeformcols_update_placements($type, $placements);
  $weight_delta = ceil(count($placements)/2);
  
  $regions = nodeformcols_form_regions();
  
  $form = array();
  $form['type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  $form['#after_build'][] = '_nodeformcols_configuration_form_after_build';
  $form['conf'] = array(
    '#theme' => array('nodeformcols_configuration'),
  );
  
  foreach ($placements as $name => $info) {
    $key = 'field_' . $name;
    $weight = isset($info['weight']) ? $info['weight'] : 0;
    $form['conf'][$info['region']][$key] = array(
      '#weight' => $weight,
      $key . '_name' => array(
        '#type' => 'markup',
        '#value' => $name,
      ),
      $key . '_region' => array(
        '#type' => 'select',
        '#options' => $regions,
        '#default_value' => $info['region'],
        '#attributes' => array(
          'class' => 'field-region-select field-region-'. $info['region'],
        ),
      ),
      $key . '_weight' => array(
        '#type' => 'weight',
        '#weight_delta' => $weight_delta,
        '#default_value' => $weight,
        '#attributes' => array(
          'class' => 'field-weight field-weight-'. $info['region'],
        ),
      ),
    );
    
    if (isset($info['collapsed'])) {
      $form['conf'][$info['region']][$key][$key . '_collapsed'] = array(
        '#type' => 'checkbox',
        '#title' => t('Show collapsed'),
        '#default_value' => $info['collapsed'],
      );
    }
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  
  return $form;
}

function _nodeformcols_configuration_form_after_build($form) {
  drupal_add_js('misc/tableheader.js');
  drupal_add_js(drupal_get_path('module', 'nodeformcols') . '/js/nodeformcols.js');
  drupal_add_css(drupal_get_path('module', 'nodeformcols') . '/css/nodeformcols.admin.css');
  
  $regions = nodeformcols_form_regions();
  foreach ($regions as $region => $title) {
    uasort($form['conf'][$region], "element_sort");
    drupal_add_tabledrag('fields', 'match', 'sibling', 'field-region-select', 'field-region-'. $region, NULL, FALSE);
    drupal_add_tabledrag('fields', 'order', 'sibling', 'field-weight', 'field-weight-'. $region);
  }
  return $form;
}

function nodeformcols_configuration_form_submit($form, $form_state) {
  $values = $form_state['values'];
  $type = $values['type'];
  $placements = nodeformscols_field_placements($type);  
  nodeformcols_update_placements($type, $placements);
  
  foreach ($placements as $name => &$opt) {
    $field = 'field_' . $name;
    $opt['region'] = $values[$field . '_region'];
    $opt['weight'] = $values[$field . '_weight'];
    if (isset($values[$field . '_collapsed'])) {
      $opt['collapsed'] = $values[$field . '_collapsed'];
    }
  }
  
  variable_set('nodeformscols_field_placements_' . $type, $placements);
}