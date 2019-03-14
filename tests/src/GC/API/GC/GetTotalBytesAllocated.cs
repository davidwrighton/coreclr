// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.
// Tests GC.GetTotalBytesAllocated(bool precise)

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;

public class Test 
{
    static Random Rand = new Random();
    static Func<bool, long> GetTotalBytesAllocated = Get_GetTotalBytesAllocatedDelegate();
    static volatile object s_stash; // static volatile variable to keep the jit from eliding allocations or anything.

    private static Func<bool, long> Get_GetTotalBytesAllocatedDelegate()
    {
        const string name = "GetTotalAllocatedBytes";
        var typeInfo = typeof(GC).GetTypeInfo();
        var method = typeInfo.GetMethod(name, BindingFlags.Static | BindingFlags.Public | BindingFlags.NonPublic);
        Func<bool, long> del = (Func<bool, long>)method.CreateDelegate(typeof(Func<bool, long>));
        // Prime the delegate to ensure its been called some.
        del(true);
        del(false);

        return del;
    }

    public static long CallGetTotalBytesAllocated(long expectMoreAllocatedThanThis, out long differenceBetweenPreciseAndImprecise)
    {
        long precise = GetTotalBytesAllocated(true);
        long imprecise = GetTotalBytesAllocated(false);
        if (precise <= 0)
        {
            Console.WriteLine("Api reported zero bytes allocated, this is unlikely.");
            throw new Exception();
        }
        if (imprecise < precise)
        {
            Console.WriteLine("Imprecise total bytes allocated less than precise, imprecise is required to be a conservative estimate (that estimates high)");
            throw new Exception();
        }
        if (expectMoreAllocatedThanThis > precise)
        {
            Console.WriteLine("Expected more memory to be allocated");
            throw new Exception();
        }
        differenceBetweenPreciseAndImprecise = imprecise-precise;
        return precise;
    }

    public static void TestSingleThreaded()
    {
        long differenceBetweenPreciseAndImprecise = 0;
        long preciseMeasure = CallGetTotalBytesAllocated(0, out differenceBetweenPreciseAndImprecise);
        s_stash = new byte[differenceBetweenPreciseAndImprecise];
        preciseMeasure = CallGetTotalBytesAllocated(preciseMeasure, out differenceBetweenPreciseAndImprecise);
    }

    public static void TestAnotherThread()
    {
        long differenceBetweenPreciseAndImprecise = 0;
        long preciseMeasure = CallGetTotalBytesAllocated(0, out differenceBetweenPreciseAndImprecise);

        Thread t = new Thread(()=>{
            for (int i = 0; i < 100; i++)
            {
                s_stash = new byte[differenceBetweenPreciseAndImprecise + 10000];
                preciseMeasure = CallGetTotalBytesAllocated(preciseMeasure, out differenceBetweenPreciseAndImprecise);
            }
        });

        t.Start();
        t.Join();
        preciseMeasure = CallGetTotalBytesAllocated(preciseMeasure, out differenceBetweenPreciseAndImprecise);
    }

    public static int Main() 
    {
        TestSingleThreaded();
        TestAnotherThread();

        const int multitasks = 1000;
        Task[] tasks = new Task[multitasks*2];
        for (int i = 0; i < multitasks; i++)
        {
            tasks[i*2] = Task.Run((Action)TestAnotherThread);
            tasks[i*2+1] = Task.Run((Action)TestSingleThreaded);
        }

        Task.WaitAll(tasks);
        return 100;
    }
}
