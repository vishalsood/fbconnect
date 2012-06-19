<?php
// $Id$

/** 
 * @file 
 * Menu callbacks for fconnect module. 
 */ 

/**
 * Logout
 */
function fconnect_logout() {
  global $base_url;
  $fbuid = fconnect_get_fb_user();
  $facebook = & fconnect_client();
  
  if ($facebook && $fbuid) {
    $facebook->logout($base_url);
  }
  
  drupal_goto('');
}
/**
 * Login
 */
function fconnect_login() {

  $fbuid = fconnect_get_fb_user();
  $next ='';
  if (!$fbuid) {
    return;
  }
  
  //Deal with destination being set on the login url
  if (isset($_REQUEST['destination'])) {
    $next = $_REQUEST['destination'];
    unset($_REQUEST['destination']);
  }
  
  $userid = fconnect_is_connected_user($fbuid);
  // If we don't have a user, register them
  if (!$userid) {
    $fconnect_allow_register_fb = variable_get('fconnect_allow_register', 1);
    if ($fconnect_allow_register_fb) {

      drupal_goto('fconnect/register');
      //TODO: Prompt user to potentially connect accounts if somehow that can be established
    }
  }
  else {
    //Login the user
    $account = user_external_load($fbuid);
    user_external_login($account);
  }

  drupal_goto($next);
}

/**
 * Render the invite friends iframe.
 */
function fconnect_render_invite_form() {
  global $base_url;
  $action = $base_url;
  $type = variable_get('site_name', '');
  $content = t('Come join me on this site');
  
  $output = '<fb:serverfbml style="width: 100%;">
    <script type="text/fbml">
      <fb:fbml>
        <fb:request-form
          action="'. $action .'"
          method="POST"
          invite="true"
          type="'. $type .'"
          content="'. $content .'  <fb:req-choice url=\''. $base_url .'\' label=\'Become a Member!\'></fb:req-choice>">	
          <fb:multi-friend-selector
          showborder="false"
          actiontext="Select the friends you want to invite.">
          </fb:multi-friend-selector>
        </fb:request-form>
      </fb:fbml>
    </script>
  </fb:serverfbml>';
  return $output;
}

/**
 * Register form
 */
function fconnect_register_page($form_state = array()) {
  $fbuid = fconnect_get_fb_user();
  if ($fbuid && !user_is_logged_in()) {
    drupal_set_title(t('Import your profile information from Facebook'));  
    //$output = drupal_get_form('fconnect_register_form');
    //return $output;
    $form  = drupal_retrieve_form('user_register', $form_state);
    drupal_prepare_form('user_register', $form, $form_state);

    //Add any default values to register form
    
    //Use name from facebook
    //Commenting this due to a facebook connect policy :(
    //$form['name']['#default_value'] = fconnect_get_fb_username($fbuid);
    
    //Whether to publish to facebook profile or not.
    $form['register']['fconnect_publish_feed'] = array(
    '#type' => 'checkbox',
    '#title' => t('Publish to Facebook'),
    '#required' => TRUE,
    '#default_value' => TRUE,
    '#description' => t('Let your friends on facebook know you joined this site')
    );
    
    //Whether to use facebook profile picture or not.
    $form['register']['fconnect_use_fb_picture'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use profile picture from Facebook'),
    '#required' => TRUE,
    '#default_value' => TRUE,
    '#description' => t('Selecting this will pull Facebook profile picture as Drupal picture')
    );
     
    $form['#submit'][] = 'fconnect_register_form_submit';  
    
    return $form;
  }
}

 /** 
  * Handle post-validation register form submission. 
  */ 
function fconnect_register_form_submit($form, &$form_state) {
  // If sucess, user_external_login_register will return NULL
  $fbuid = fconnect_get_fb_user();
  $username = $form_state['values']['name'];
  $email = $form_state['values']['mail'];
  
  //The user is already created by this time.
  $user = user_load(array('name' => $username));
  $user_id = $user->uid;
  
  //Create a Drupal user for the current user
  db_query("INSERT INTO {authmap} (authname, uid, module) VALUES ('%s', %d, '%s')", $fbuid, $user_id, 'fconnect');
  
  user_external_login_register($username, 'fconnect');

  //Register the user with facebook.
  $result = fconnect_register_user($user_id, $email);
     
  // In case of register false
  if ($result != NULL) {
    watchdog('fconnect', 'User external register failed for %user.', array('%user' => $form_state['values']['fconnect_username']));
    $form_state['redirect'] = '';
    return;
  }
    
  //Publish story to facebook
  $publish_flag = $form_state['values']['fconnect_publish_feed'];
  if ($publish_flag) {
    //display feed dialog
    $_SESSION['fconnect_feed_prompt']['type'] = 'register';
  }
    
  // redirect to invite friends page
  $redirect_path = 'fconnect/invite';
  $form_state['redirect'] = $redirect_path;
}

/**
 * This registers use with Drupal and facebook.
 * @param $fbid
 * @param $username
 * @param $email
 * @param $modulename
 * @return unknown_type
 */
function fconnect_register_user($user_id, $email ) {
  //Validate changes to the hash
  $email_hash = fconnect_email_get_public_hash($email);
  $accounts = array(array('email_hash' => $email_hash, 'account_id' => $user_id, 'account_url' => 'http://vishalsood.com/users/'. $user_id));
  //We have separate method so that durpal and facebook code can be segregated
  fconnect_register_users($accounts);
}

function fconnect_unregister_user($email ) {
  //Validate changes to the hash
  $email_hash = fconnect_email_get_public_hash($email);
  //We have separate method so that drupal and facebook code can be segregated
  fconnect_unregister_users($email_hash);
}

/**
 * Menu callback; Manage Facebook identity for the specified user.
 */
function fconnect_user_identities($account) {
  drupal_set_title(check_plain($account->name));
  $fbuid = fconnect_get_fb_user();
  $current_user = $account->uid;
  $logged_in_user = $GLOBALS['user']->uid;
  $connected_fb_user =  fconnect_is_drupal_user_connected($current_user);

  $output = '<div class="fconnect_linkedAccount">';
  //Check and ensure that the user is not already connected?
  if (!$connected_fb_user) {
    if ($current_user == $logged_in_user) {
      if ($fbuid && $fbuid > 0) {
        //Is a facebook user logged in?
        //show connect option
        $output .= '<div>'. _fconnect_render_profile_pic($fbuid, 'square', 'true') .'</div>';
        $output .= '<div><fb:name uid="'. $fbuid .'" useyou="false" linked="true"></fb:name></div>';
        if (!fconnect_is_connected_user($fbuid)) {// ensure the FB user is not already connected
          $output .= drupal_get_form('fconnect_assoc_acc_form'); 
        }
        else {
          $output .='This account is already connected to another user';
        }
      }
      else { 
        //show facebook button
        $output .= '<fb:login-button size="medium" background="light" length="long"></fb:login-button>';
      }
    }
  }
  else { //already connected user
    //TODO: Enable connecting multiple facebook profiles also
    $result = db_query("SELECT * FROM {authmap} WHERE module='fconnect' AND uid=%d", $account->uid);
    while ($identity = db_fetch_object($result)) {
      $output .= '<div>'. _fconnect_render_profile_pic($identity->authname, 'normal', 'true') .'</div>';
      $output .= '<div><fb:name uid="'. $identity->authname .'" useyou="false" linked="true" ></fb:name></div>';
      if ($fbuid == $connected_fb_user) { // TODO: Verify logic here
        $output .= drupal_get_form('fconnect_assoc_del_form');
      }
    }
  }
  $output .= '</div>';
  return $output;
}

/*
 * This functin is for displaying the association form
 */
function fconnect_assoc_acc_form() {
   $fbuid = fconnect_get_fb_user();
   //TODO: Figure out how to display user info inside the form
   $form['profileId'] = array(
     '#type' => 'hidden',
     '#value' => $fbuid,
   );
   $form['submit'] = array( 
     '#type' => 'submit', 
     '#value' => t('Associate'), 
   );
   return $form;
}

/*
 * This functin is for the association form submit
 */
function fconnect_assoc_acc_form_submit($form, &$form_state) {
  $fbuid = $form_state['values']['profileId'];
  $drupal_user_id = $GLOBALS['user']->uid;
  $email = $GLOBALS['user']->mail;
  if ($fbuid && $fbuid > 0) {
    $result = db_query("INSERT INTO {authmap} (uid, authname, module) VALUES (%d, '%s','fconnect')", $drupal_user_id, $fbuid);
    if ($result) {
      drupal_set_message('Your account was successfully associated. You can now use your facebook credentials on this site. ');
      //TODO: Handle failure to register with Facebook.
      fconnect_register_user($drupal_user_id, $email);
    }
  }
  else {
    $redirect_path = '';
    $form_state['redirect'] = $redirect_path;
    form_set_error('fconnect_assoc_acc_form', t('Association Failed'));
  }
}

/*
 * This function is for displaying the form to delete association
 */
function fconnect_assoc_del_form() {
  $form['submit'] = array(
  '#type' => 'submit',
  '#value' => t('Remove Association'),
  );
  return $form;
}

/*
 * This function is for deleting association on form submit
 */
function fconnect_assoc_del_form_submit($form, &$form_state) {
  //TODO: Is global user correct here?
  $drupal_user_id = $GLOBALS['user']->uid;
  //Ensure only currently logged in facebook users are able to remove
  $fbuid = fconnect_get_fb_user();
  
  $result = db_query("DELETE FROM {authmap} WHERE uid= %d and authname = '%s' AND module='fconnect'", $drupal_user_id, $fbuid);
  if ($result) {
    drupal_set_message('Your Facebook account association was successfully removed.');
    $email = $GLOBALS['user']->mail;
    fconnect_unregister_user($email_hash);
  }
}

// End of file