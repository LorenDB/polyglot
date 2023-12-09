// SPDX-FileCopyrightText: Matheus Catarino França
//
// SPDX-License-Identifier: GPL-3.0

const std = @import("std");
const ast = std.zig.Ast;
const Token = std.zig.Token;
const Tokenizer = std.zig.Tokenizer;

// some std.zig.Tokenizer tests
// https://github.com/ziglang/zig/blob/master/lib/std/zig/tokenizer.zig

test "Tokenizer tests" {
    try testTokenize("const a = 1;", &[_]Token.Tag{
        Token.Tag.keyword_const,
        Token.Tag.identifier,
        Token.Tag.equal,
        Token.Tag.number_literal,
        Token.Tag.semicolon,
        Token.Tag.eof,
    });

    try testTokenize("const a = 1.0;var b = &a;", &[_]Token.Tag{
        Token.Tag.keyword_const,
        Token.Tag.identifier,
        Token.Tag.equal,
        Token.Tag.number_literal,
        Token.Tag.semicolon,
        Token.Tag.keyword_var,
        Token.Tag.identifier,
        Token.Tag.equal,
        Token.Tag.ampersand,
        Token.Tag.identifier,
        Token.Tag.semicolon,
        Token.Tag.eof,
    });

    try testTokenize("const a = \"text\";", &[_]Token.Tag{
        Token.Tag.keyword_const,
        Token.Tag.identifier,
        Token.Tag.equal,
        Token.Tag.string_literal,
        Token.Tag.semicolon,
        Token.Tag.eof,
    });
}

fn testTokenize(source: [:0]const u8, expected_token_tags: []const Token.Tag) !void {
    var tokenizer = Tokenizer.init(source);
    for (expected_token_tags) |expected_token_tag| {
        const token = tokenizer.next();
        try std.testing.expectEqual(expected_token_tag, token.tag);
    }
    const last_token = tokenizer.next();
    try std.testing.expectEqual(Token.Tag.eof, last_token.tag);
    try std.testing.expectEqual(source.len, last_token.loc.start);
    try std.testing.expectEqual(source.len, last_token.loc.end);
}
