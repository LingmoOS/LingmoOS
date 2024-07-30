set(server_top_srcs
    .htaccess
)

set(server_admin_srcs
    admin/index.php
    admin/.htaccess
)

set(server_analytics_srcs
    analytics/index.php
    analytics/.htaccess
)

set(server_config_srcs
    config/localconfig.php.example
)

set(server_shared_srcs
    shared/admin.php
    shared/aggregation.php
    shared/analytics.php
    shared/compat.php
    shared/config.php
    shared/datastore.php
    shared/product.php
    shared/receiver.php
    shared/restdispatcher.php
    shared/restexception.php
    shared/sample.php
    shared/schema.json
    shared/schemaentry.php
    shared/schemaentryelement.php
    shared/survey.php
    shared/utils.php
)

set(server_receiver_srcs
    receiver/index.php
    receiver/.htaccess
)

php_lint(${server_admin_srcs})
php_lint(${server_analytics_srcs})
php_lint(${server_config_srcs})
php_lint(${server_shared_srcs})
php_lint(${server_receiver_srcs})

if (SERVER_DEPLOY_HOST AND SERVER_DEPLOY_PATH)
    set(SERVER_DEPLOY_DESTINATION ${SERVER_DEPLOY_HOST}:${SERVER_DEPLOY_PATH})
    add_custom_target(deploy
        COMMAND ssh ${SERVER_DEPLOY_HOST} "mkdir -p ${SERVER_DEPLOY_PATH}"
        COMMAND scp ${server_top_srcs} ${SERVER_DEPLOY_DESTINATION}/
        COMMAND ssh ${SERVER_DEPLOY_HOST} "mkdir -p ${SERVER_DEPLOY_PATH}/admin"
        COMMAND scp ${server_admin_srcs} ${SERVER_DEPLOY_DESTINATION}/admin
        COMMAND ssh ${SERVER_DEPLOY_HOST} "mkdir -p ${SERVER_DEPLOY_PATH}/analytics"
        COMMAND scp ${server_analytics_srcs} ${SERVER_DEPLOY_DESTINATION}/analytics
        COMMAND ssh ${SERVER_DEPLOY_HOST} "mkdir -p ${SERVER_DEPLOY_PATH}/config"
        COMMAND scp ${server_config_srcs} ${SERVER_DEPLOY_DESTINATION}/config
        COMMAND ssh ${SERVER_DEPLOY_HOST} "mkdir -p ${SERVER_DEPLOY_PATH}/receiver"
        COMMAND scp ${server_receiver_srcs} ${SERVER_DEPLOY_DESTINATION}/receiver
        COMMAND ssh ${SERVER_DEPLOY_HOST} "mkdir -p ${SERVER_DEPLOY_PATH}/shared"
        COMMAND scp ${server_shared_srcs} ${SERVER_DEPLOY_DESTINATION}/shared
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()
