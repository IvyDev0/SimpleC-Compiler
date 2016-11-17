/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     INTEGER = 258,
     FLOAT = 259,
     OCT = 260,
     HEX = 261,
     TYPE = 262,
     STRUCT = 263,
     RETURN = 264,
     IF = 265,
     ELSE = 266,
     WHILE = 267,
     ID = 268,
     SPACE = 269,
     SEMI = 270,
     COMMA = 271,
     ASSIGNOP = 272,
     RELOP = 273,
     PLUS = 274,
     MINUS = 275,
     STAR = 276,
     DIV = 277,
     AND = 278,
     OR = 279,
     DOT = 280,
     NOT = 281,
     LP = 282,
     RP = 283,
     LB = 284,
     RB = 285,
     LC = 286,
     RC = 287,
     AERROR = 288,
     EOL = 289,
     LOWER_THAN_ELSE = 290
   };
#endif
/* Tokens.  */
#define INTEGER 258
#define FLOAT 259
#define OCT 260
#define HEX 261
#define TYPE 262
#define STRUCT 263
#define RETURN 264
#define IF 265
#define ELSE 266
#define WHILE 267
#define ID 268
#define SPACE 269
#define SEMI 270
#define COMMA 271
#define ASSIGNOP 272
#define RELOP 273
#define PLUS 274
#define MINUS 275
#define STAR 276
#define DIV 277
#define AND 278
#define OR 279
#define DOT 280
#define NOT 281
#define LP 282
#define RP 283
#define LB 284
#define RB 285
#define LC 286
#define RC 287
#define AERROR 288
#define EOL 289
#define LOWER_THAN_ELSE 290




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 5 "parser.y"
{
struct ast* a;
double d;
}
/* Line 1529 of yacc.c.  */
#line 124 "parser.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

