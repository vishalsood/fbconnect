<?php
// $Id: fconnect.admin.inc,v 1.0 2009/01/09 09:19:18 vsood Exp $

/** 
 * @file 
 * Administration page callbacks for the fbconnect module. 
 */ 

/** 
 * Form builder. Configure fbconnect. 
 * 
 * @ingroup forms 
 * @see system_settings_form(). 
 */
function fconnect_admin_settings() {
   $form['fconnect_api_config'] = array(
      '#type' => 'fieldset',
      '#title' => t('Facebook API settings'),
      '#collapsible' => FALSE,
   );
   // Facebook api key
   $form['fconnect_api_config']['facebook_api_key'] = array( 
      '#type' => 'textfield', 
      '#title' => t('Facebook API KEY'), 
      '#default_value' => variable_get('facebook_api_key', ''), 
   );   
   // Facebook  secret
   $form['fconnect_api_config']['facebook_secret'] = array(
      '#type' => 'textfield',
      '#title' => t('Facebook Secret'),
      '#default_value' => variable_get('facebook_secret', ''),
   ); 
   $form['fconnect_api_config']['fconnect_allow_register'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow users to register using Facebook Credentials?'),
      '#default_value' => variable_get('fconnect_allow_register', 1),
      '#description' => t('If checked, this option allows user to register new accounts using Facebook credentials'),
   );
      $form['fconnect_api_config']['fconnect_show_login_button'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show the Facebook Connect Login button in Login Block'),
      '#default_value' => variable_get('fconnect_show_login_button', 1),
      '#description' => t('If checked, Login button will be visible in the login block'),
   );  
   $form['fconnect_api_config']['facebook_button_size'] = array(
      '#type' => 'select',
      '#title' => t('Facebook Button Size'),
      '#default_value' => variable_get('facebook_button_size', 'medium'),
      '#options' => array('small', 'medium', 'large'),
   );
   $form['fconnect_api_config']['facebook_button_length'] = array(
      '#type' => 'select',
      '#title' => t('Facebook Login Button Length'),
      '#default_value' => variable_get('facebook_button_length', 'short'),
      '#options' => array('short', 'long'),
   );
   $form['fconnect_api_config']['facebook_profile_pic_size'] = array(
      '#type' => 'select',
      '#title' => t('Facebook Profile picture size'),
      '#default_value' => variable_get('facebook_profile_pic_size', 'normal'),
      '#options' => array('thumb', 'small', 'normal', 'square'),
   );
   
   $form['fconnect_api_config']['fconnect_share_button'] = array(
      '#type' => 'fieldset',
      '#title' => t('Facebook Share Button settings.'),
      '#description' => t('NOTE: These settings are applicable to only content type where this button is enabled.'),
      '#collapsible' => TRUE,
   );
   $form['fconnect_api_config']['fconnect_share_button']['facebook_share_button_big'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show Big Facebook Share button?'),
      '#default_value' => variable_get('facebook_share_button_big', 1),
      '#description' => t('Show a big box_count button at the beginning of content for Full View for supported nodes'),
   ); 
   //This is to show the action link at the bottom or not
   $form['fconnect_api_config']['fconnect_share_button']['facebook_share_button_visibility'] = array(
      '#type' => 'select',
      '#title' => t('Show Facebook Share Action Link for'),
      '#default_value' => variable_get('facebook_share_button_visibility', 'full'),
      '#options' => array('teaser', 'full', 'both', 'none'),
   );
   // facebook_share_button is facebook_share_button_type
   $form['fconnect_api_config']['fconnect_share_button']['facebook_share_button_type'] = array(
      '#type' => 'select',
      '#title' => t('Facebook Share Action Link Type'),
      '#default_value' => variable_get('facebook_share_button_type', 'button_count'),
      '#options' => array('box_count', 'button_count', 'button', 'icon_link', 'icon'),
   ); 
//   $form['fconnect_api_config']['fconnect_share_button']['facebook_share_button_weight'] = array(
//      '#type' => 'select',
//      '#title' => t('Show Facebook Share before/after content (full node view only)'),
//      '#default_value' => variable_get('facebook_share_button_weight', 'before'),
//      '#options' => array('before', 'after'),
//   );
//   $form['fconnect_api_config']['fconnect_share_button']['facebook_share_button_teaser'] = array(
//      '#type' => 'select',
//      '#title' => t('Facebook Share Button Type for Teasers'),
//      '#default_value' => variable_get('facebook_share_button_teaser', 'button_count'),
//      '#options' => array('box_count', 'button_count','button','icon_link','icon'),
//   );     
   return system_settings_form($form); 
}
//End of file