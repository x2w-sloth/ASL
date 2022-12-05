if exists("b:current_syntax")
  finish
endif

syn match aslIdentifier '[a-zA-Z0-9_]\+' contained
syn match aslNominalType '[a-zA-Z0-9_]\+' contained
syn match aslComment "//.*$" contains=aslTodo
syn keyword aslTodo TODO contained

" asl primitive types and literals
syn keyword aslBooleanType bool
syn match aslIntType 'i\d\+'
syn match aslUnsignedIntType 'u\d\+'
syn match aslFloatType 'f\d\+'
syn keyword aslBoolean true false
syn match aslNumber '\<[0-9][_0-9]*\(\.[_0-9]\+\(e[-+]\?[1-9][0-9]*\)\?\)\?\>'
syn region aslStringLiteral start=+"+ end=+"+ skip=+\\"+

" asl declaration introducers
syn keyword aslScopeDeclaration scope
syn keyword aslFunctionDeclaration fn
syn keyword aslAliasDeclaration alias nextgroup=aslNominalType skipwhite

" asl control flow
syn keyword aslConditional if else
syn keyword aslLoop for
syn keyword aslControlFlowStatement return

" asl operators
syn keyword aslLogicalOperator and or not

hi def link aslIdentifier Identifier
hi def link aslNominalType Type
hi def link aslComment Comment
hi def link aslTodo Todo
hi def link aslBooleanType aslType
hi def link aslIntType aslType
hi def link aslUnsignedIntType aslType
hi def link aslFloatType aslType
hi def link aslType Type
hi def link aslBoolean Boolean
hi def link aslNumber Number
hi def link aslStringLiteral aslString
hi def link aslString String
hi def link aslScopeDeclaration aslDeclaration
hi def link aslFunctionDeclaration aslDeclaration
hi def link aslAliasDeclaration aslDeclaration
hi def link aslDeclaration Structure
hi def link aslConditional Conditional
hi def link aslLoop Repeat
hi def link aslControlFlowStatement Statement
hi def link aslLogicalOperator aslOperator
hi def link aslOperator Operator

let b:current_syntax = "asl"
