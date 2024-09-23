#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]] || [[ "$1" != "placement" && "$1" != "replacement" ]]; then
    echo "Please specify 'placement' or 'replacement'" >&2
    exit 1
fi

LOG="$(mktemp)"
echo "Logging to $LOG" >&2

run_cli_placement() {
    start="$(date '+%s')"
    if ./cli/cli -VV reverse -L 1 -m 1073741824 --kernel-cpu 10 --kernel-isolation "$2" --pmu intel placement -S "$1" -P "$3" -R "$4" > "$LOG" 2>&1; then
        end="$(date '+%s')"

        echo -n "$1,"
        echo -n "$2,"
        echo -n "$3,"
        echo -n "$4,"
        echo -n "$((end - start)),"
        fgrep "bits for reversal of index function" "$LOG" | perl -pe 's/.*?(\d+) bits.*/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Index function" "$LOG" | perl -pe 's/.*= (.*)/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Placement policy confidence" "$LOG" | perl -pe 's/.*?(\d+)\/\d+/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Accesses:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Instrumented accesses:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Flushes:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Channel switches:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo ""
    else
        echo "Error, check log at $LOG" >&2
        return 1
    fi
}

run_all_cisw_cli_placement() {
    for isolation in no-preempt off disable-irq; do
        for i in $(seq 1 10); do
            while true; do
                if run_cli_placement cisw "$isolation" 0 1; then
                    break
                fi
            done
        done
    done
}

run_all_evset_cli_placement() {
    for isolation in disable-irq; do
        for pollution in 50000; do
            for reps in 2; do
                for i in $(seq 1 10); do
                    run_cli_placement eviction-set "$isolation" "$pollution" "$reps"
                done
            done
        done
    done
}

run_cli_replacement() {
    start="$(date '+%s')"
    if ./cli/cli -VV reverse -L 2 -m 1073741824 --kernel-cpu 16 --kernel-isolation "$1" --pmu intel replacement -P "$2" -R "$3" -i 37 > "$LOG" 2>&1; then
        end="$(date '+%s')"

        echo -n "$1,"
        echo -n "$2,"
        echo -n "$3,"
        echo -n "$((end - start)),"
        fgrep "Found replacement policy:" "$LOG" 2>&1 | cut -d ':' -f4 | tr -d ' \n'
        echo -n ","
        fgrep "Accesses:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Instrumented accesses:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Flushes:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo -n ","
        fgrep "Channel switches:" "$LOG" 2>&1 | perl -pe 's/^.*?([0-9]+)$/\1/' | tr -d '\n'
        echo ""
    else
        echo "Error, check log at $LOG" >&2
        return 1
    fi
}

run_multi_cli_replacement() {
    for i in $(seq 1 100); do
        while true; do
            if run_cli_replacement no-preempt 10000 10; then
                break
            fi
        done
    done
}

if [[ "$1" == "placement" ]]; then
    echo strategy,isolation,pollution,reps,time,bits,func,confidence,accesses,instrumented_accesses,flushes,channel_switches

    # run_all_cisw_cli_placement
    # run_all_evset_cli_placement
elif [[ "$1" == "replacement" ]]; then
    echo isolation,pollution,reps,time,policy,accesses,instrumented_accesses,flushes,channel_switches

    run_multi_cli_replacement
fi
