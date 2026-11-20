program ::= { declaration }

declaration ::= IDENT ':' declaration_tail

declaration_tail ::= variable_declaration_tail
                   | function_declaration_tail

variable_declaration_tail ::= type [ '=' initializer ] ';'

function_declaration_tail ::= 'function' return_type
                              '(' [ param_list ] ')'
                              function_body

function_body ::= ';'
                | '=' block

param_list ::= param { ',' param }

param ::= IDENT ':' parameter_type

type ::= atomic_type | array_type

atomic_type ::= 'integer'
              | 'boolean'
              | 'char'
              | 'string'

array_type ::= 'array' '[' INT_LIT ']' atomic_type

parameter_type ::= atomic_type
                  | 'array' '[' ']' atomic_type

return_type ::= atomic_type
              | 'void'

initializer ::= expression
              | array_initializer

array_initializer ::= '{' expression { ',' expression } '}'

block ::= '{' { block_item } '}'

block_item ::= local_declaration
             | statement

local_declaration ::= IDENT ':' variable_declaration_tail

expression_statement ::= expression ';'

return_statement ::= 'return' [ expression ] ';'

expression_list ::= expression { ',' expression }

print_statement ::= 'print' expression_list ';'

if_statement ::= 'if' '(' expression ')' statement
                 [ 'else' statement ]

for_statement ::= 'for' '(' [ expression ] ';'
                  [ expression ] ';'
                  [ expression ] ')' 
                  statement

statement ::= expression_statement
            | return_statement
            | print_statement
            | if_statement
            | for_statement
            | block

expression ::= assignment_expression

assignment_expression ::= logical_expression [ '=' assignment_expression ]

logical_expression ::= equality_expression { ('&&' | '||') equality_expression }

equality_expression ::= relational_expression
{ ('==' | '!=') relational_expression }

relational_expression ::= additive_expression
                          { ('<' | '<=' | '>' | '>=') additive_expression }

additive_expression ::= multiplicative_expression
                        { ('+' | '-') multiplicative_expression }

multiplicative_expression ::= power_expression
                              { ('*' | '/' | '%') power_expression }

power_expression ::= unary_expression { '^' unary_expression }

unary_expression ::= ('!' | '-') unary_expression | postfix_expression

postfix_expression ::= primary_expression
                       { '(' [argument_list] ')'
                       | '[' expression ']'
                       | '++'
                       | '--' }

argument_list ::= expression { ',' expression }

primary_expression ::= IDENT
                     | INT_LIT
                     | CHAR_LIT
                     | STRING_LIT
                     | 'true'
                     | 'false'
                     | '(' expression ')'


## تحلیل گرامر و سازگاری با تحلیلگر نحوی

روش تحلیل نحوی پروژه **بازگشتی نزولی (LL)** است و برای هر غیرپایانه‌ی گرامر یک تابع در parser نوشته می‌شود. بخش‌های زیر نشان می‌دهند گرامر بالا با این روش سازگار است.

### ۱. بازگشت چپ

این گرامر بازگشت چپ (مستقیم یا غیرمستقیم) ندارد. در همه‌ی سطح‌های عبارت، به‌جای قاعده‌ی بازگشت‌چپ `A ::= A op B` از الگوی تکرار `A ::= B { op B }` استفاده شده است؛ مثلاً در `additive_expression`. بنابراین نیازی به حذف بازگشت چپ نیست و هر قاعده مستقیماً به یک تابع بازگشتی نزولی تبدیل می‌شود.

### ۲. نیاز به دو توکن پیش‌نگری (LL(2)) در `block_item`

در قاعده‌ی `block_item`، هر دو گزینه‌ی `local_declaration` (مثل `x: integer;`) و `statement` (مثل `x = 5;`) می‌توانند با `IDENT` شروع شوند، پس با یک توکن پیش‌نگری نمی‌توان یکی را انتخاب کرد. parser توکن دوم را هم نگاه می‌کند: اگر `:` باشد، یک اعلان محلی است و در غیر این صورت یک دستور. این تنها نقطه‌ای در گرامر است که LL(1) کافی نیست و بقیه‌ی گرامر LL(1) است.

### ۳. مبهم بودن else معلق (dangling else)

قاعده‌ی `if_statement` از نظر صوری مبهم است: در `if (a) if (b) x; else y;` هم می‌توان `else` را به `if` داخلی چسباند و هم به `if` بیرونی. این تعارض، یک تعارض FOLLOW است، چون `else` عضو FOLLOW غیرپایانه‌ی `statement` هم هست. قاعده‌ی انتخاب‌شده این است که **`else` به نزدیک‌ترین `if` که `else` ندارد می‌چسبد**. در پیاده‌سازی، بعد از تجزیه‌ی دستور داخلی `if`، اگر توکن بعدی `else` باشد، parser حریصانه آن را همان‌جا مصرف می‌کند و این قاعده خودبه‌خود اعمال می‌شود.