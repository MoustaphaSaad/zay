# Zay
The pragmatic programming language

# Session 02

Well, let me explain what i'm doing
Compilers usually is done in this steps

Frontend:
scan(source text) -> Tokens
parse(Tokens) -> AST(Abstract Syntax Tree)
type_check(AST) -> Annotated AST

Backend:
Generic Optimization:
ast_optimize(AST) -> AST
ssa_generate(AST) -> SSA(Static Single Assignment)
ssa_optimize(SSA) -> SSA

Machine Specific Optimizations:
assembly_lowering(SSA) -> Assembly
assembly_optimize(Assembly) -> Assembly

now we need to list our tokens but first let's get a feel for the language

```
//this is a struct which only contain data
type Point struct {
	x, y: f32
}

//this is how we declare variables
var x: int
//here's a declaration assignment combo
var x = 2354 //the type is inferred
var x: int = 234 //the type is declared explicitly
//let's add ; as need in the parser which we'll do in a later time

func add(a, b: Point): Point {
	return Point{
		x: a.x + b.x,
		y: a.y + b.y
	}
}
```
let's remember what we did last time
we were able to parse the syntax i guess and was about to do type checking
let's do it then