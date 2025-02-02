/*
 * Copyright (c) 2020, the SerenityOS developers.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "UCICommand.h"
#include <AK/StringBuilder.h>

namespace Chess::UCI {

UCICommand UCICommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "uci");
    VERIFY(tokens.size() == 1);
    return UCICommand();
}

DeprecatedString UCICommand::to_deprecated_string() const
{
    return "uci\n";
}

DebugCommand DebugCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "debug");
    VERIFY(tokens.size() == 2);
    if (tokens[1] == "on")
        return DebugCommand(Flag::On);
    if (tokens[1] == "off")
        return DebugCommand(Flag::On);

    VERIFY_NOT_REACHED();
}

DeprecatedString DebugCommand::to_deprecated_string() const
{
    if (flag() == Flag::On) {
        return "debug on\n";
    } else {
        return "debug off\n";
    }
}

IsReadyCommand IsReadyCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "isready");
    VERIFY(tokens.size() == 1);
    return IsReadyCommand();
}

DeprecatedString IsReadyCommand::to_deprecated_string() const
{
    return "isready\n";
}

SetOptionCommand SetOptionCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "setoption");
    VERIFY(tokens[1] == "name");
    VERIFY(tokens.size() > 2);

    StringBuilder name;
    StringBuilder value;
    bool in_name = false;
    bool in_value = false;
    for (auto& part : tokens) {
        if (in_name) {
            if (part == "value") {
                in_name = false;
                in_value = true;
                continue;
            }
            name.append(part);
            name.append(' ');
            continue;
        }
        if (in_value) {
            value.append(part);
            value.append(' ');
            continue;
        }
        if (part == "name") {
            in_name = true;
            continue;
        }
    }

    VERIFY(!name.is_empty());

    return SetOptionCommand(name.to_deprecated_string().trim_whitespace(), value.to_deprecated_string().trim_whitespace());
}

DeprecatedString SetOptionCommand::to_deprecated_string() const
{
    StringBuilder builder;
    builder.append("setoption name "sv);
    builder.append(name());
    if (value().has_value()) {
        builder.append(" value "sv);
        builder.append(value().value());
    }
    builder.append('\n');
    return builder.to_deprecated_string();
}

PositionCommand PositionCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens.size() >= 3);
    VERIFY(tokens[0] == "position");
    VERIFY(tokens[2] == "moves");

    Optional<DeprecatedString> fen;
    if (tokens[1] != "startpos")
        fen = tokens[1];

    Vector<Move> moves;
    for (size_t i = 3; i < tokens.size(); ++i) {
        moves.append(Move(tokens[i]));
    }
    return PositionCommand(fen, moves);
}

DeprecatedString PositionCommand::to_deprecated_string() const
{
    StringBuilder builder;
    builder.append("position "sv);
    if (fen().has_value()) {
        builder.append(fen().value());
    } else {
        builder.append("startpos "sv);
    }
    builder.append("moves"sv);
    for (auto& move : moves()) {
        builder.append(' ');
        builder.append(move.to_long_algebraic());
    }
    builder.append('\n');
    return builder.to_deprecated_string();
}

GoCommand GoCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "go");

    GoCommand go_command;
    for (size_t i = 1; i < tokens.size(); ++i) {
        if (tokens[i] == "searchmoves") {
            VERIFY_NOT_REACHED();
        } else if (tokens[i] == "ponder") {
            go_command.ponder = true;
        } else if (tokens[i] == "wtime") {
            VERIFY(i++ < tokens.size());
            go_command.wtime = tokens[i].to_int().value();
        } else if (tokens[i] == "btime") {
            VERIFY(i++ < tokens.size());
            go_command.btime = tokens[i].to_int().value();
        } else if (tokens[i] == "winc") {
            VERIFY(i++ < tokens.size());
            go_command.winc = tokens[i].to_int().value();
        } else if (tokens[i] == "binc") {
            VERIFY(i++ < tokens.size());
            go_command.binc = tokens[i].to_int().value();
        } else if (tokens[i] == "movestogo") {
            VERIFY(i++ < tokens.size());
            go_command.movestogo = tokens[i].to_int().value();
        } else if (tokens[i] == "depth") {
            VERIFY(i++ < tokens.size());
            go_command.depth = tokens[i].to_int().value();
        } else if (tokens[i] == "nodes") {
            VERIFY(i++ < tokens.size());
            go_command.nodes = tokens[i].to_int().value();
        } else if (tokens[i] == "mate") {
            VERIFY(i++ < tokens.size());
            go_command.mate = tokens[i].to_int().value();
        } else if (tokens[i] == "movetime") {
            VERIFY(i++ < tokens.size());
            go_command.movetime = tokens[i].to_int().value();
        } else if (tokens[i] == "infinite") {
            go_command.infinite = true;
        }
    }

    return go_command;
}

DeprecatedString GoCommand::to_deprecated_string() const
{
    StringBuilder builder;
    builder.append("go"sv);

    if (searchmoves.has_value()) {
        builder.append(" searchmoves"sv);
        for (auto& move : searchmoves.value()) {
            builder.append(' ');
            builder.append(move.to_long_algebraic());
        }
    }

    if (ponder)
        builder.append(" ponder"sv);
    if (wtime.has_value())
        builder.appendff(" wtime {}", wtime.value());
    if (btime.has_value())
        builder.appendff(" btime {}", btime.value());
    if (winc.has_value())
        builder.appendff(" winc {}", winc.value());
    if (binc.has_value())
        builder.appendff(" binc {}", binc.value());
    if (movestogo.has_value())
        builder.appendff(" movestogo {}", movestogo.value());
    if (depth.has_value())
        builder.appendff(" depth {}", depth.value());
    if (nodes.has_value())
        builder.appendff(" nodes {}", nodes.value());
    if (mate.has_value())
        builder.appendff(" mate {}", mate.value());
    if (movetime.has_value())
        builder.appendff(" movetime {}", movetime.value());
    if (infinite)
        builder.append(" infinite"sv);

    builder.append('\n');
    return builder.to_deprecated_string();
}

StopCommand StopCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "stop");
    VERIFY(tokens.size() == 1);
    return StopCommand();
}

DeprecatedString StopCommand::to_deprecated_string() const
{
    return "stop\n";
}

IdCommand IdCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "id");
    StringBuilder value;
    for (size_t i = 2; i < tokens.size(); ++i) {
        if (i != 2)
            value.append(' ');

        value.append(tokens[i]);
    }

    if (tokens[1] == "name") {
        return IdCommand(Type::Name, value.to_deprecated_string());
    } else if (tokens[1] == "author") {
        return IdCommand(Type::Author, value.to_deprecated_string());
    }
    VERIFY_NOT_REACHED();
}

DeprecatedString IdCommand::to_deprecated_string() const
{
    StringBuilder builder;
    builder.append("id "sv);
    if (field_type() == Type::Name) {
        builder.append("name "sv);
    } else {
        builder.append("author "sv);
    }
    builder.append(value());
    builder.append('\n');
    return builder.to_deprecated_string();
}

UCIOkCommand UCIOkCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "uciok");
    VERIFY(tokens.size() == 1);
    return UCIOkCommand();
}

DeprecatedString UCIOkCommand::to_deprecated_string() const
{
    return "uciok\n";
}

ReadyOkCommand ReadyOkCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "readyok");
    VERIFY(tokens.size() == 1);
    return ReadyOkCommand();
}

DeprecatedString ReadyOkCommand::to_deprecated_string() const
{
    return "readyok\n";
}

BestMoveCommand BestMoveCommand::from_string(StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "bestmove");
    VERIFY(tokens.size() == 2);
    return BestMoveCommand(Move(tokens[1]));
}

DeprecatedString BestMoveCommand::to_deprecated_string() const
{
    StringBuilder builder;
    builder.append("bestmove "sv);
    builder.append(move().to_long_algebraic());
    builder.append('\n');
    return builder.to_deprecated_string();
}

InfoCommand InfoCommand::from_string([[maybe_unused]] StringView command)
{
    // FIXME: Implement this.
    VERIFY_NOT_REACHED();
}

DeprecatedString InfoCommand::to_deprecated_string() const
{
    // FIXME: Implement this.
    VERIFY_NOT_REACHED();
    return "info";
}

QuitCommand QuitCommand::from_string([[maybe_unused]] StringView command)
{
    auto tokens = command.split_view(' ');
    VERIFY(tokens[0] == "quit");
    VERIFY(tokens.size() == 1);
    return QuitCommand();
}

DeprecatedString QuitCommand::to_deprecated_string() const
{
    return "quit\n";
}

}
