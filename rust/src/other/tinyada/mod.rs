use alloc::{string::String, vec::Vec};



#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    NONE, NUMBER, IDENT, PLUS, MINUS, STRING,
    DOT, SEMICOLON
}

#[derive(Debug, Clone)]
pub struct Token {
    pub token_type: TokenType,
    pub value: String,
    pub start_pos: usize,
    pub end_pos: usize,
    pub line: usize
}

pub struct Ada<'a> {
    data: &'a [u8],
    buffer: String,
    size: usize,
    pos: usize,
    old_pos: usize,
    line: usize,
    pub tokens: Vec<Token>
}

impl<'a> Ada<'a> {
    pub fn new(data: &'a [u8]) -> Self {
        Self {
            data,
            buffer: String::new(),
            size: data.len(),
            pos: 0,
            old_pos: 0,
            line: 1,
            tokens: Vec::new()
        }
    }

    fn number(&mut self) {
        while self.pos < self.size {
            let ch = self.data[self.pos] as char;
            if ch.is_numeric() {
                self.buffer.push(ch);
                self.pos += 1;
            }
            else if ch.is_alphabetic() || ['_'].contains(&ch) {
                self.ident();
                return;
            } else {
                break;
            }
        }

        let token = Token {
            token_type: TokenType::NUMBER,
            value: self.buffer.clone(),
            start_pos: self.old_pos,
            end_pos: self.pos,
            line: self.line
        };
        self.buffer = String::new();
        self.tokens.push(token);
    }

    fn ident(&mut self) {
        while self.pos < self.size {
            let ch = self.data[self.pos] as char;
            if ch.is_alphanumeric() || ['_'].contains(&ch) {
                self.buffer.push(ch);
                self.pos += 1;
            } else {
                break;
            }
        }
        let token = Token {
            token_type: TokenType::IDENT,
            value: self.buffer.clone(),
            start_pos: self.old_pos,
            end_pos: self.pos,
            line: self.line
        };
        self.buffer = String::new();
        self.tokens.push(token);
    }

    fn none(&mut self) -> Token {
        let token = Token {
            token_type: TokenType::NONE,
            value: String::new(),
            start_pos: self.old_pos,
            end_pos: self.pos,
            line: self.line
        };
        token
    }

    pub fn lexer(&mut self) {
        self.tokens = Vec::new();
        while self.pos < self.size {
            self.old_pos = self.pos;
            match self.data[self.pos] as char {
                '0'..='9' => {
                    self.number();
                }
                'a'..='z' | 'A'..='Z' => {
                    self.ident();
                }
                '+' => {
                    let mut token = self.none();
                    token.token_type = TokenType::PLUS;
                    token.value.push('+');

                    self.tokens.push(token);
                    self.pos += 1;
                }
                '-' => {
                    let mut token = self.none();
                    token.token_type = TokenType::MINUS;
                    token.value.push('-');

                    self.tokens.push(token);
                    self.pos += 1;
                },
                '.' => {
                    let mut token = self.none();
                    token.token_type = TokenType::DOT;
                    token.value.push('.');

                    self.tokens.push(token);
                    self.pos += 1;
                }
                ';' => {
                    let mut token = self.none();
                    token.token_type = TokenType::SEMICOLON;
                    token.value.push(';');

                    self.tokens.push(token);
                    self.pos += 1;
                }
                '\n' => {
                    self.line += 1;
                    self.pos += 1;
                }
                _ => {
                    self.pos += 1;
                }
            }
        }
    }
}