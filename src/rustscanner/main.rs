// SPDX-FileCopyrightText: Loren Burkholder
//
// SPDX-License-Identifier: GPL-3.0

// use rustc_ast::ast;
// use rustc_ast::parse::ParseSess;
// use rustc_ast::visit::{self, Visitor};
// use rustc_symbol_mangling::{mangle, SymbolManglingVersion};

// // Define a custom visitor that implements the Visitor trait
// struct Mangler<'a> {
//     sess: &'a ParseSess,
// }

// impl<'a> Mangler<'a> {
//     // Create a new Mangler with a reference to a ParseSess
//     fn new(sess: &'a ParseSess) -> Self {
//         Self { sess }
//     }
// }

// impl<'a> Visitor<'a> for Mangler<'a> {
//     // Override the visit_item method to handle function items
//     fn visit_item(&mut self, item: &'a ast::Item) {
//         // Check if the item is a function
//         if let ast::ItemKind::Fn(..) = item.kind {
//             // Get the function name as a string
//             let name = item.ident.to_string();
//             // Encode the function name using the v0 mangling scheme
//             let mangled = mangle(&name, SymbolManglingVersion::V0).unwrap();
//             // Print the original and mangled names
//             println!("{} -> {}", name, mangled);
//         }
//         // Recursively visit the nested items
//         visit::walk_item(self, item);
//     }
// }

// fn main() {
//     // Create a new ParseSess
//     let sess = ParseSess::new_default();
//     // Parse a Rust source file into an AST
//     let file = "src/main.rs";
//     let ast = sess.parse_file_to_ast(file).unwrap();
//     // Create a new Mangler
//     let mut mangler = Mangler::new(&sess);
//     // Visit the AST and print the mangled function names
//     mangler.visit_crate(&ast);
// }

#![feature(rustc_private)]
extern crate rustc_ast;
extern crate rustc_parse;
extern crate rustc_session;
extern crate rustc_symbol_mangling;

use rustc_ast::ast;
use rustc_session::parse::ParseSess;

fn main() 
{
    let file = "/home/loren/code/polyglot/tests/test3.rs";
}
