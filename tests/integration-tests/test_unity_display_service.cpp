/*
 * Copyright © 2015 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#include "src/unity_display_service.h"
#include "src/dbus_connection_handle.h"
#include "src/dbus_connection_thread.h"
#include "src/dbus_event_loop.h"
#include "src/dbus_message_handle.h"
#include "src/screen.h"
#include "src/unity_display_service_introspection.h"
#include "wait_condition.h"
#include "dbus_bus.h"
#include "dbus_client.h"
#include "unity_display_dbus_client.h"

#include "usc/test/mock_screen.h"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <stdexcept>
#include <memory>

namespace ut = usc::test;

namespace
{

struct FakeScreen : ut::MockScreen
{
    void register_active_outputs_handler(usc::ActiveOutputsHandler const& handler)
    {
        std::lock_guard<std::mutex> lock{active_outputs_mutex};
        active_outputs_handler = handler;
    }

    void notify_active_outputs(usc::ActiveOutputs const& active_outputs)
    {
        std::lock_guard<std::mutex> lock{active_outputs_mutex};
        active_outputs_handler(active_outputs);
    }

    std::mutex active_outputs_mutex;
    usc::ActiveOutputsHandler active_outputs_handler{[](usc::ActiveOutputs const&){}};
};

struct AUnityDisplayService : testing::Test
{
    ut::DBusBus bus;

    std::shared_ptr<FakeScreen> const fake_screen =
        std::make_shared<testing::NiceMock<FakeScreen>>();
    ut::UnityDisplayDBusClient client{bus.address()};
    std::shared_ptr<usc::DBusEventLoop> const dbus_loop =
        std::make_shared<usc::DBusEventLoop>();
    usc::UnityDisplayService service{dbus_loop, bus.address(), fake_screen};
    std::shared_ptr<usc::DBusConnectionThread> const dbus_thread =
        std::make_shared<usc::DBusConnectionThread>(dbus_loop);
};

}

TEST_F(AUnityDisplayService, replies_to_introspection_request)
{
    using namespace testing;

    auto reply = client.request_introspection();
    EXPECT_THAT(reply.get(), Eq(unity_display_service_introspection));
}

TEST_F(AUnityDisplayService, forwards_turn_on_request)
{
    EXPECT_CALL(*fake_screen, turn_on());

    client.request_turn_on();
}

TEST_F(AUnityDisplayService, forwards_turn_off_request)
{
    EXPECT_CALL(*fake_screen, turn_off());

    client.request_turn_off();
}

TEST_F(AUnityDisplayService, emits_num_active_displays)
{
    using namespace testing;

    usc::ActiveOutputs expected_active_outputs;
    expected_active_outputs.internal = 2;
    expected_active_outputs.external = 3;

    fake_screen->notify_active_outputs(expected_active_outputs);
    // Received messages are queued at the destination, so it doesn't
    // matter that we start listening after the signal has been sent
    auto message = client.listen_for_num_active_outputs_signal();

    usc::ActiveOutputs active_outputs{-1, -1};
    dbus_message_get_args(message, nullptr,
        DBUS_TYPE_INT32, &active_outputs.internal,
        DBUS_TYPE_INT32, &active_outputs.external,
        DBUS_TYPE_INVALID);

    EXPECT_THAT(active_outputs, Eq(expected_active_outputs));
}

TEST_F(AUnityDisplayService, returns_error_reply_for_unsupported_method)
{
    using namespace testing;

    auto reply = client.request_invalid_method();
    auto reply_msg = reply.get();

    EXPECT_THAT(dbus_message_get_type(reply_msg), Eq(DBUS_MESSAGE_TYPE_ERROR));
    EXPECT_THAT(dbus_message_get_error_name(reply_msg), StrEq(DBUS_ERROR_FAILED));
}
