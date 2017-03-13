; ModuleID = 'test.bc'
source_filename = "test.bc"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [20 x i8] c"\0AEnter the string :\00", align 1
@.str.1 = private unnamed_addr constant [23 x i8] c"\0AReverse string is :%s\00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %str = alloca [100 x i8], align 16
  %temp = alloca i8, align 1
  %i = alloca i32, align 4
  %j = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* %j, align 4
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str, i32 0, i32 0))
  %3 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i32 0, i32 0
  %4 = call i32 (i8*, ...) bitcast (i32 (...)* @gets to i32 (i8*, ...)*)(i8* %3)
  store i32 0, i32* %i, align 4
  %5 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i32 0, i32 0
  %6 = call i64 @strlen(i8* %5) #3
  %7 = sub i64 %6, 1
  %8 = trunc i64 %7 to i32
  store i32 %8, i32* %j, align 4
  br label %9

; <label>:9:                                      ; preds = %13, %0
  %10 = load i32, i32* %i, align 4
  %11 = load i32, i32* %j, align 4
  %12 = icmp slt i32 %10, %11
  br i1 %12, label %13, label %33

; <label>:13:                                     ; preds = %9
  %14 = load i32, i32* %i, align 4
  %15 = sext i32 %14 to i64
  %16 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i64 0, i64 %15
  %17 = load i8, i8* %16, align 1
  store i8 %17, i8* %temp, align 1
  %18 = load i32, i32* %j, align 4
  %19 = sext i32 %18 to i64
  %20 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i64 0, i64 %19
  %21 = load i8, i8* %20, align 1
  %22 = load i32, i32* %i, align 4
  %23 = sext i32 %22 to i64
  %24 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i64 0, i64 %23
  store i8 %21, i8* %24, align 1
  %25 = load i8, i8* %temp, align 1
  %26 = load i32, i32* %j, align 4
  %27 = sext i32 %26 to i64
  %28 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i64 0, i64 %27
  store i8 %25, i8* %28, align 1
  %29 = load i32, i32* %i, align 4
  %30 = add nsw i32 %29, 1
  store i32 %30, i32* %i, align 4
  %31 = load i32, i32* %j, align 4
  %32 = add nsw i32 %31, -1
  store i32 %32, i32* %j, align 4
  br label %9

; <label>:33:                                     ; preds = %9
  %34 = getelementptr inbounds [100 x i8], [100 x i8]* %str, i32 0, i32 0
  %35 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.1, i32 0, i32 0), i8* %34)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

declare i32 @gets(...) #1

; Function Attrs: nounwind readonly
declare i64 @strlen(i8*) #2

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readonly "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readonly }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (branches/release_38 259745) (llvm/branches/release_38 259685)"}
