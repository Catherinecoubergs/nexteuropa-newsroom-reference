<?php

/**
 * @file
 * Code for the NewsroomPage class.
 */

/**
 * NewsroomPage class.
 */
class NewsroomPage {

  const DATE_FORMAT = 'd/m/Y';
  const MYSQL_DATE_FORMAT = 'd-m-Y';

  protected $type = NULL;
  protected $topic = NULL;
  protected $startDate = NULL;
  protected $endDate = NULL;
  protected $ecContent = NEXTEUROPA_NEWSROOM_EC;

  /**
   * NewsroomPage constructor.
   *
   * @param object|null $type
   *   Newsroom type.
   * @param object|null $topic
   *   Newsroom topic.
   * @param Datetime|null $start_date
   *   Start date.
   * @param Datetime|null $end_date
   *   End date.
   * @param string $ec_content
   *   EC content.
   */
  public function __construct($type = NULL, $topic = NULL, $start_date = NULL, $end_date = NULL, $ec_content = NEXTEUROPA_NEWSROOM_EC) {
    $this->setTopic($topic);
    $this->setType($type);
    $this->setStartDate($start_date);
    $this->setEndDate($end_date);
    $this->setEcContent($ec_content);
  }

  /**
   * Gets minimal items number per agenda page.
   *
   * @return int
   *   Number of items.
   */
  public function getItemsNumber() {
    return variable_get('newsroom_page_items_number', 10);
  }

  /**
   * Gets term URL.
   *
   * @param string $value
   *   URL part.
   *
   * @return string
   *   Term url part.
   */
  public function getTermUrl($value) {
    return !empty($value) ? pathauto_cleanstring($value) : NEXTEUROPA_NEWSROOM_ALL;
  }

  /**
   * Sets type.
   *
   * @param object|null $value
   *   Type object.
   */
  public function setType($value) {
    $this->type = $value;
  }

  /**
   * Gets type.
   *
   * @return object|null
   *   Type object.
   */
  public function getType() {
    return $this->type;
  }

  /**
   * Sets start date.
   *
   * @param DateTime|null $value
   *   Start date.
   */
  public function setStartDate($value) {
    $this->startDate = $value;
  }

  /**
   * Gets start date.
   *
   * @return DateTime|null
   *   Start date.
   */
  public function getStartDate() {
    return $this->startDate;
  }

  /**
   * Sets end date.
   *
   * @param DateTime|null $value
   *   End date.
   */
  public function setEndDate($value) {
    $this->endDate = $value;
  }

  /**
   * Gets end date.
   *
   * @return DateTime|null
   *   End date.
   */
  public function getEndDate() {
    return $this->endDate;
  }

  /**
   * Sets topic.
   *
   * @param object|null $value
   *   Topic object.
   */
  public function setTopic($value) {
    $this->topic = $value;
  }

  /**
   * Gets topic.
   *
   * @return object|null
   *   Topic object.
   */
  public function getTopic() {
    return $this->topic;
  }

  /**
   * Sets EC content.
   *
   * @param string $value
   *   EC content.
   */
  public function setEcContent($value) {
    $this->ecContent = $value;
  }

  /**
   * Gets EC content.
   *
   * @return string
   *   EC content.
   */
  public function getEcContent() {
    return $this->ecContent;
  }

  /**
   * Gets image formatted output.
   *
   * @param object $node
   *   Node.
   * @param string $field_name
   *   Field name.
   * @param string $style_name
   *   Style name.
   *
   * @return null|string
   *   Image HTML presentation.
   */
  protected function getNodeImage($node, $field_name, $style_name = 'newsroom_style') {
    $image = NULL;
    if ($style_name == 'newsroom_style') {
      $display = [
        'type' => 'newsroom_zoomable_image',
        'settings' => _nexteuropa_newsroom_zoomable_formatter_settings('newsroom_style'),
      ];
    }
    else {
      $display = [
        'type' => 'image',
        'settings' => [
          'image_style' => 'newsroom_style',
          'image_link' => 'content',
        ],
      ];
    }

    $field_image = field_view_field('node', $node, $field_name, $display);
    if ($field_image) {
      $field_image['#label_display'] = 'hidden';
      $image = drupal_render($field_image);
    }

    return $image;
  }

  /**
   * Creates formated date string.
   *
   * @param string $timestamp
   *   Timestamp.
   *
   * @return string
   *   Formated string.
   */
  protected function getFormatedDateFromTimestamp($timestamp) {
    return $timestamp ? NewsroomHelper::getFormatedDate(NewsroomPage::DATE_FORMAT, $timestamp) : NULL;
  }

  /**
   * Gets data from DB.
   */
  protected function getData() {
    $key_hash_part = $this->getUniqueCachePart();
    $cache_key = 'page_result:' . $key_hash_part;
    $cache_total_number_key = 'page_total_number:' . $key_hash_part;
    if ($cache = cache_get($cache_key, NEXTEUROPA_NEWSROOM_CACHE_TABLE)) {
      // For cached result, we set manually params for pager.
      if ($total_number_cache = cache_get($cache_total_number_key, NEXTEUROPA_NEWSROOM_CACHE_TABLE)) {
        pager_default_initialize($total_number_cache->data, $this->getItemsNumber());
      }
      $data = $cache->data;
    }
    else {
      $query = $this->getQueryObject();
      cache_set($cache_total_number_key, $query->extend('PagerDefault')->getCountQuery()->execute()->fetchField(), NEXTEUROPA_NEWSROOM_CACHE_TABLE, time() + variable_get('newsroom_ct_page_result', 300));
      $items = $query->extend('PagerDefault')->limit($this->getItemsNumber())->execute()->fetchAll();
      $data = [];
      if ($items) {
        foreach ($items as $item) {
          $node = node_load($item->nid);
          $type = NewsroomHelper::getNewsroomType($item->type_tid);
          $short_title = NewsroomHelper::getFieldTranslatedValue($node, 'field_newsroom_item_short_title');

          $image = $this->getNodeImage($node, 'field_newsroom_illustrative_img');
          if (empty($image)) {
            $image = $this->getNodeImage($node, 'field_newsroom_private_img');
          }

          $data[] = [
            'nid' => $item->nid,
            'start_date' => $this->getFormatedDateFromTimestamp($item->start_date),
            'end_date' => $this->getFormatedDateFromTimestamp($item->end_date),
            'title' => $short_title ? $short_title : $node->title,
            'teaser' => NewsroomHelper::getFieldTranslatedValue($node, 'field_newsroom_teaser'),
            'type_url' => NewsroomHelper::getNewsroomPageUrl([$type ? pathauto_cleanstring($type->name) : NEXTEUROPA_NEWSROOM_ALL, NEXTEUROPA_NEWSROOM_ALL]),
            'type_title' => $type ? $type->name : NULL,
            'node_url' => url(NewsroomHelper::getNodeMainUrl($node), ['absolute' => TRUE]),
            'node_redirect_url' => url('news-redirect/' . NewsroomHelper::getFieldValue($node, 'field_newsroom_item_id'), ['absolute' => TRUE]),
            'image' => $image,
            'venue' => NewsroomHelper::getFieldTranslatedValue($node, 'field_newsroom_item_venue'),
          ];
        }
      }

      cache_set($cache_key, $data, NEXTEUROPA_NEWSROOM_CACHE_TABLE, time() + variable_get('newsroom_ct_page_result', 300));
    }

    return $data;
  }

  /**
   * Returns the newsroom page html.
   *
   * @return string
   *   HTML.
   */
  public function getContent() {
    drupal_set_title($this->generateTitle());
    $filter_form = drupal_get_form('nexteuropa_newsroom_page_filter_form', $this->getType(), $this->getTopic(), $this->getStartDate(), $this->getEndDate(), $this->getEcContent());
    $content = theme('newsroom_page', [
      'filter_form' => drupal_render($filter_form),
      'featured_item' => $this->getTypeFeaturedItem(),
      'items' => $this->generateItems('newsroom_page_item'),
    ]);
    return $content;
  }

  /**
   * Returns the newsroom page html.
   *
   * @return string
   *   HTML.
   */
  public function getRssContent() {
    $namespaces = [];
    if (function_exists('rdf_get_namespaces')) {
      // Merge RDF namespaces in the XML namespaces in case they are used
      // further in the RSS content.
      $xml_rdf_namespaces = [];
      foreach (rdf_get_namespaces() as $prefix => $uri) {
        $xml_rdf_namespaces['xmlns:' . $prefix] = $uri;
      }
      $namespaces += $xml_rdf_namespaces;
    }

    $content = theme('newsroom_page_rss', [
      'title' => $this->generateTitle(),
      'link' => url($this->getNavigationUrl(TRUE), ['absolute' => TRUE]),
      'namespaces' => drupal_attributes($namespaces),
      'description' => NULL,
      'items' => $this->generateItems('newsroom_page_rss_item'),
    ]);
    return $content;
  }

  /**
   * Generates title based on incoming parameters.
   *
   * @return null|string
   *   Page title.
   */
  protected function generateTitle() {
    $page_title = t('Newsroom');
    if ($this->getType()) {
      $page_title = $this->getType()->name;
    }
    if ($this->getTopic()) {
      $page_title .= ' ' . t('about !topic_title', ['!topic_title' => $this->getTopic()->name]);
    }
    return $page_title;
  }

  /**
   * Prepares page items for presenation.
   *
   * @param string $template_name
   *   Template name.
   *
   * @return string
   *   Output string.
   */
  protected function generateItems($template_name) {
    $output = NULL;
    foreach ($this->getData() as $item) {
      $output .= theme($template_name, $item);
    }
    return $output;
  }

  /**
   * Gets the unique part for cache value.
   *
   * @return string
   *   Cache key unique value.
   */
  protected function getUniqueCachePart() {
    $type_id = $this->getType() ? $this->getType()->tid : NULL;
    $topic_id = $this->getTopic() ? $this->getTopic()->tid : NULL;
    $start_date = $this->getStartDate() ? $this->getStartDate()->getTimestamp() : NULL;
    $end_date = $this->getEndDate() ? $this->getEndDate()->getTimestamp() : NULL;
    $ec_content = $this->getEcContent();
    $page = !empty($_GET['page']) ? $_GET['page'] : NULL;

    return md5($type_id . $topic_id . $start_date . $end_date . $page . $ec_content);
  }

  /**
   * Gets query object.
   */
  public function getQueryObject() {
    $query = db_select('node', 'n');
    $query->distinct();
    $query->fields('n', ['nid', 'created']);
    $query->addField('type_field', 'field_newsroom_item_type_tid', 'type_tid');
    $query->addField('d', 'field_newsroom_item_date_value', 'start_date');
    $query->addField('d', 'field_newsroom_item_date_value2', 'end_date');

    $query->leftJoin('field_data_field_newsroom_item_type', 'type_field', 'type_field.entity_id = n.nid');
    $query->leftJoin('field_data_field_newsroom_topics', 'topic_field', 'topic_field.entity_id = n.nid');
    $query->leftJoin('field_data_field_newsroom_item_date', 'd', 'd.entity_id = n.nid');
    $query->leftJoin('field_data_field_newsroom_ec_content', 'ec', 'ec.entity_id = n.nid');
    $query->leftJoin('field_data_field_newsroom_newsletter_only', 'newsletter_item', 'newsletter_item.entity_id = n.nid');

    $query->condition('n.status', 1);
    $query->condition('n.type', NEXTEUROPA_NEWSROOM_CONTENT_TYPE);
    $query->condition('ec.field_newsroom_ec_content_value', $this->getEcContent() == NEXTEUROPA_NEWSROOM_EC ? 1 : 0);

    $db_or = db_or();
    $db_or->condition('newsletter_item.field_newsroom_newsletter_only_value', 1, '<>');
    $db_or->isNull('newsletter_item.field_newsroom_newsletter_only_value');

    $query->condition($db_or);

    $query->orderBy('start_date', 'DESC');
    $query->orderBy('end_date', 'DESC');

    if ($this->getType()) {
      $query->condition('type_field.field_newsroom_item_type_tid', NewsroomHelper::getTypeChildren($this->getType()->tid));
    }

    if ($this->getTopic()) {
      $query->condition('topic_field.field_newsroom_topics_tid', $this->getTopic()->tid);
    }

    if ($this->getStartDate()) {
      $query->condition('n.created', $this->getStartDate()->getTimestamp(), '>=');
    }

    if ($this->getEndDate()) {
      $query->condition('n.created', $this->getEndDate()->getTimestamp(), '<=');
    }

    $query->addTag('node_access');

    return $query;
  }

  /**
   * Returns navigation link based on URL.
   *
   * @param bool $is_rss
   *   Is url for RSS feed or not.
   *
   * @return string|null
   *   HTML link.
   */
  public function getNavigationUrl($is_rss = FALSE) {
    $url_parts = [];
    $url_parts[] = $this->getTermUrl($this->getType() ? $this->getType()->name : NULL);
    $url_parts[] = $this->getTermUrl($this->getTopic() ? $this->getTopic()->name : NULL);
    $url_parts[] = $this->getStartDate() ? $this->getStartDate()->format(NewsroomPage::MYSQL_DATE_FORMAT) : NEXTEUROPA_NEWSROOM_ALL;
    $url_parts[] = $this->getEndDate() ? $this->getEndDate()->format(NewsroomPage::MYSQL_DATE_FORMAT) : NEXTEUROPA_NEWSROOM_ALL;
    $url_parts[] = $this->getEcContent() == NEXTEUROPA_NEWSROOM_EC ? NEXTEUROPA_NEWSROOM_EC : NEXTEUROPA_NEWSROOM_NONEC;

    return $is_rss ? NewsroomHelper::getNewsroomRssUrl($url_parts) : NewsroomHelper::getNewsroomPageUrl($url_parts);
  }

  /**
   * Gets type featured item.
   *
   * @return null|string
   *   Featured item content.
   */
  protected function getTypeFeaturedItem() {
    $content = NULL;
    $featured_item_id = NewsroomHelper::getFieldValue($this->getType(), 'field_newsroom_featured_item', 'target_id');
    if (empty($featured_item_id)) {
      // If featured item is not set we try to take from feature newsroom type.
      if ($featured_type = NewsroomHelper::getNewsroomType(variable_get('newsroom_featured_type'))) {
        $featured_item_id = NewsroomHelper::getFieldValue($featured_type, 'field_newsroom_featured_item', 'target_id');
      }
    }

    if ($featured_item_id) {
      $cache_key = 'type_featured_item:' . $this->getType()->tid;

      if ($cache = cache_get($cache_key, NEXTEUROPA_NEWSROOM_CACHE_TABLE)) {
        $content = $cache->data;
      }
      else {

        if ($featured_node = node_load($featured_item_id)) {

          $type = NewsroomHelper::getNewsroomType(NewsroomHelper::getFieldValue($featured_node, 'field_newsroom_item_type', 'tid'));

          $image_style = 'large';
          $image = $this->getNodeImage($featured_node, 'field_newsroom_illustrative_img', $image_style);
          if (empty($image)) {
            $image = $this->getNodeImage($featured_node, 'field_newsroom_private_img', $image_style);
          }

          $topic_url = $this->getTermUrl($this->getTopic() ? $this->getTopic()->name : NULL);

          $content = theme('newsroom_page_featured_item', [
            'type' => $type ? l($type->name, NewsroomHelper::getNewsroomPageUrl([$this->getTermUrl($type->name), $topic_url])) : NULL,
            'title' => l($featured_node->title, 'node/' . $featured_node->nid),
            'image' => $image,
            'created' => NewsroomHelper::getFormatedDate(NewsroomPage::DATE_FORMAT, $featured_node->created),
            'teaser' => NewsroomHelper::getFieldTranslatedValue($featured_node, 'field_newsroom_teaser'),
          ]);
        }

        cache_set($cache_key, $content, NEXTEUROPA_NEWSROOM_CACHE_TABLE, time() + variable_get('newsroom_ct_type_featured_item', 300));

      }
    }

    return $content;
  }

}
