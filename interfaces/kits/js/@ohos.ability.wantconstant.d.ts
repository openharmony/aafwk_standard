/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * the constant for action and entity in the want
 * @name wantConstant
 * @since 3
 * @sysCap aafwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace wantConstant {
  /**
   * the constant for action of the want
   * @name Action
   * @since 3
   * @sysCap aafwk
   * @devices phone, tablet
   * @permission N/A
   */
  export enum Action {
    /**
     * Indicates the action of backing home.
     *
     * @since 1
     */
    ACTION_HOME = "action.system.home",

    /**
     * Indicates the action of playing a media item.
     *
     * @since 1
     */
    ACTION_PLAY = "action.system.play",

    /**
     * Indicates the action of installing a bundle.
     *
     * @since 1
     */
    ACTION_BUNDLE_ADD = "action.bundle.add",

    /**
     * Indicates the action of uninstalling a bundle.
     *
     * @since 1
     */
    ACTION_BUNDLE_REMOVE = "action.bundle.remove",

    /**
     * Indicates the action of updating a bundle.
     *
     * @since 1
     */
    ACTION_BUNDLE_UPDATE = "action.bundle.update",

    /**
     * Indicates the action of booking a taxi.
     *
     * @since 1
     */
    ACTION_ORDER_TAXI = "ability.intent.ORDER_TAXI",

    /**
     * Indicates the action of querying driving bans.
     *
     * @since 1
     */
    ACTION_QUERY_TRAFFIC_RESTRICTION = "ability.intent.QUERY_TRAFFIC_RESTRICTION",

    /**
     * Indicates the action of querying routes.
     *
     * @since 1
     */
    ACTION_PLAN_ROUTE = "ability.intent.PLAN_ROUTE",

    /**
     * Indicates the action of booking a flight.
     *
     * @since 1
     */
    ACTION_BOOK_FLIGHT = "ability.intent.BOOK_FLIGHT",

    /**
     * Indicates the action of booking a train ticket.
     *
     * @since 1
     */
    ACTION_BOOK_TRAIN_TICKET = "ability.intent.BOOK_TRAIN_TICKET",

    /**
     * Indicates the action of booking a hotel.
     *
     * @since 1
     */
    ACTION_BOOK_HOTEL = "ability.intent.BOOK_HOTEL",

    /**
     * Indicates the action of querying a travel guide.
     *
     * @since 1
     */
    ACTION_QUERY_TRAVELLING_GUIDELINE = "ability.intent.QUERY_TRAVELLING_GUIDELINE",

    /**
     * Indicates the action of querying nearby places.
     *
     * @since 1
     */
    ACTION_QUERY_POI_INFO = "ability.intent.QUERY_POI_INFO",

    /**
     * Indicates the action of querying horoscope.
     *
     * @since 1
     */
    ACTION_QUERY_CONSTELLATION_FORTUNE = "ability.intent.QUERY_CONSTELLATION_FORTUNE",

    /**
     * Indicates the action of querying the Chinese Almanac calendar.
     *
     * @since 1
     */
    ACTION_QUERY_ALMANC = "ability.intent.QUERY_ALMANC",

    /**
     * Indicates the action of querying weather.
     *
     * @since 1
     */
    ACTION_QUERY_WEATHER = "ability.intent.QUERY_WEATHER",

    /**
     * Indicates the action of querying the encyclopedia.
     *
     * @since 1
     */
    ACTION_QUERY_ENCYCLOPEDIA = "ability.intent.QUERY_ENCYCLOPEDIA",

    /**
     * Indicates the action of searching for a recipe.
     *
     * @since 1
     */
    ACTION_QUERY_RECIPE = "ability.intent.QUERY_RECIPE",

    /**
     * Indicates the action of ordering take-out food.
     *
     * @since 1
     */
    ACTION_BUY_TAKEOUT = "ability.intent.BUY_TAKEOUT",

    /**
     * Indicates the action of translating text.
     *
     * @since 1
     */
    ACTION_TRANSLATE_TEXT = "ability.intent.TRANSLATE_TEXT",

    /**
     * Indicates the action of shopping.
     *
     * @since 1
     */
    ACTION_BUY = "ability.intent.BUY",

    /**
     * Indicates the action of tracking shipment status.
     *
     * @since 1
     */
    ACTION_QUERY_LOGISTICS_INFO = "ability.intent.QUERY_LOGISTICS_INFO",

    /**
     * Indicates the action of shipping a bundle.
     *
     * @since 1
     */
    ACTION_SEND_LOGISTICS = "ability.intent.SEND_LOGISTICS",

    /**
     * Indicates the action of querying sports teams and game schedule.
     *
     * @since 1
     */
    ACTION_QUERY_SPORTS_INFO = "ability.intent.QUERY_SPORTS_INFO",

    /**
     * Indicates the action of browsing news.
     *
     * @since 1
     */
    ACTION_QUERY_NEWS = "ability.intent.QUERY_NEWS",

    /**
     * Indicates the action of reading jokes.
     *
     * @since 1
     */
    ACTION_QUERY_JOKE = "ability.intent.QUERY_JOKE",

    /**
     * Indicates the action of watching video clips.
     *
     * @since 1
     */
    ACTION_WATCH_VIDEO_CLIPS = "ability.intent.WATCH_VIDEO_CLIPS",

    /**
     * Indicates the action of querying stock market quotes.
     *
     * @since 1
     */
    ACTION_QUERY_STOCK_INFO = "ability.intent.QUERY_STOCK_INFO",

    /**
     * Indicates that the locale has changed.
     *
     * @since 1
     */
    ACTION_LOCALE_CHANGED = "ability.intent.LOCALE_CHANGED"
  }

  /**
   * the constant for Entity of the want
   * @name Action
   * @since 3
   * @sysCap aafwk
   * @devices phone, tablet
   * @permission N/A
   */
  export enum Entity {
    /**
     * Indicates the home screen category.
     *
     * @since 1
     */
    ENTITY_HOME = "entity.system.home",

    /**
     * Indicates the video category.
     *
     * @since 1
     */
    ENTITY_VIDEO = "entity.system.video"
  }
}

export default wantConstant;