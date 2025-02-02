/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "UCIEndpoint.h"
#include <AK/ByteBuffer.h>
#include <AK/Debug.h>
#include <AK/DeprecatedString.h>
#include <LibCore/EventLoop.h>

namespace Chess::UCI {

Endpoint::Endpoint(NonnullRefPtr<Core::IODevice> in, NonnullRefPtr<Core::IODevice> out)
    : m_in(in)
    , m_out(out)
    , m_in_notifier(Core::Notifier::construct(in->fd(), Core::Notifier::Read))
{
    set_in_notifier();
}

void Endpoint::send_command(Command const& command)
{
    dbgln_if(UCI_DEBUG, "{} Sent UCI Command: {}", class_name(), DeprecatedString(command.to_deprecated_string().characters(), Chomp));
    m_out->write(command.to_deprecated_string());
}

void Endpoint::event(Core::Event& event)
{
    switch (static_cast<Command::Type>(event.type())) {
    case Command::Type::UCI:
        return handle_uci();
    case Command::Type::Debug:
        return handle_debug(static_cast<DebugCommand const&>(event));
    case Command::Type::IsReady:
        return handle_uci();
    case Command::Type::SetOption:
        return handle_setoption(static_cast<SetOptionCommand const&>(event));
    case Command::Type::Position:
        return handle_position(static_cast<PositionCommand const&>(event));
    case Command::Type::Go:
        return handle_go(static_cast<GoCommand const&>(event));
    case Command::Type::Stop:
        return handle_stop();
    case Command::Type::Id:
        return handle_id(static_cast<IdCommand const&>(event));
    case Command::Type::UCIOk:
        return handle_uciok();
    case Command::Type::ReadyOk:
        return handle_readyok();
    case Command::Type::BestMove:
        return handle_bestmove(static_cast<BestMoveCommand const&>(event));
    case Command::Type::Info:
        return handle_info(static_cast<InfoCommand const&>(event));
    case Command::Type::Quit:
        return handle_quit();
    default:
        Object::event(event);
        break;
    }
}

void Endpoint::custom_event(Core::CustomEvent& custom_event)
{
    if (custom_event.custom_type() == EndpointEventType::UnexpectedEof)
        handle_unexpected_eof();
}

void Endpoint::set_in_notifier()
{
    m_in_notifier = Core::Notifier::construct(m_in->fd(), Core::Notifier::Read);
    m_in_notifier->on_ready_to_read = [this] {
        if (!m_in->can_read_line()) {
            Core::EventLoop::current().post_event(*this, make<Core::CustomEvent>(EndpointEventType::UnexpectedEof));
            m_in_notifier->set_enabled(false);
            return;
        }

        while (m_in->can_read_line())
            Core::EventLoop::current().post_event(*this, read_command());
    };
}

NonnullOwnPtr<Command> Endpoint::read_command()
{
    DeprecatedString line(ReadonlyBytes(m_in->read_line(4096).bytes()), Chomp);

    dbgln_if(UCI_DEBUG, "{} Received UCI Command: {}", class_name(), line);

    if (line == "uci") {
        return make<UCICommand>(UCICommand::from_string(line));
    } else if (line.starts_with("debug"sv)) {
        return make<DebugCommand>(DebugCommand::from_string(line));
    } else if (line.starts_with("isready"sv)) {
        return make<IsReadyCommand>(IsReadyCommand::from_string(line));
    } else if (line.starts_with("setoption"sv)) {
        return make<SetOptionCommand>(SetOptionCommand::from_string(line));
    } else if (line.starts_with("position"sv)) {
        return make<PositionCommand>(PositionCommand::from_string(line));
    } else if (line.starts_with("go"sv)) {
        return make<GoCommand>(GoCommand::from_string(line));
    } else if (line.starts_with("stop"sv)) {
        return make<StopCommand>(StopCommand::from_string(line));
    } else if (line.starts_with("id"sv)) {
        return make<IdCommand>(IdCommand::from_string(line));
    } else if (line.starts_with("uciok"sv)) {
        return make<UCIOkCommand>(UCIOkCommand::from_string(line));
    } else if (line.starts_with("readyok"sv)) {
        return make<ReadyOkCommand>(ReadyOkCommand::from_string(line));
    } else if (line.starts_with("bestmove"sv)) {
        return make<BestMoveCommand>(BestMoveCommand::from_string(line));
    } else if (line.starts_with("info"sv)) {
        return make<InfoCommand>(InfoCommand::from_string(line));
    } else if (line.starts_with("quit"sv)) {
        return make<QuitCommand>(QuitCommand::from_string(line));
    }

    dbgln("command line: {}", line);
    VERIFY_NOT_REACHED();
}

};
