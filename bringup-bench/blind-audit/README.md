# Certified Blind Audit Selection

This benchmark asks an untrusted server to choose one of eight audit records.
For candidate `i`, the server computes `CERTRNG(i) XOR request_nonce`, then uses
unsigned encrypted comparisons and conditional moves to select the smallest
priority. The selected ID stays encrypted until the client presents a datagrant
whose `dfhash` names the exact expected computation.

The simulator argument `--mojov-arg=N` selects one standalone test case. Tests
0, 1, and 2 are expected to finish successfully. Tests 10 through 17 intentionally
produce the wrong dataflow receipt and are expected to terminate with a Mojo-V
security exception when the datagrant is tested.

## Test cases

### 0 — Honest certified selection (positive)

The server draws from sites 0 through 7 exactly once, XORs every draw with the
fresh branded nonce, and runs all seven argmin comparisons. The returned receipt
matches the independently constructed client receipt, so validation succeeds;
the benchmark rechecks that exact receipt immediately before its client-side
debug context decrypts the selected record.
Strict less-than comparisons retain the lower candidate index if priorities tie.

### 1 — One hundred unused draws (positive unused-draw control)

The server executes 100 additional CERTRNG sites, discards them, and then runs
the honest sampler. Unused instructions are not ancestors of the returned ID,
so its receipt remains valid. Extra hidden randomness alone is not grinding;
the server needs an outcome-classification oracle to choose a favorable run.

### 2 — Disclose before commitment (positive unsafe-disclosure control)

The benchmark first performs visible plaintext attempts, then repeatedly runs a
fully valid certified selection while disclosing every winner to the server. In
both modes the server can retry until record 5 wins (up to 50 attempts here).
The computations and proofs remain valid, demonstrating that CERTRNG alone does
not prevent grinding if a random-dependent result is revealed before commitment.

### 10 — Force target 5 (negative)

Candidate 5's key is replaced with zero. This can strongly bias the numerical
selection toward the server's preferred record, but removes CERTRNG site 5 and
its nonce XOR from the selected ID's ancestry. The honest datagrant rejects it.

### 11 — Software-controlled randomness (negative)

The server generates candidate 3's value with plaintext `rand()` and loads it
into a secret register instead of invoking CERTRNG site 3. The attacker cannot
assign a client brand to that value. It can look random numerically, but its
ordinary-load provenance is not certified-random provenance, so the receipts
differ.

### 12 — Skip the request nonce (negative)

Candidate 4 uses a genuine site-4 draw directly. Omitting the XOR means that
candidate is not bound to the current request nonce, so the missing operation
and nonce brand cause proof verification to fail.

### 13 — Replay an old nonce (negative)

All eight otherwise-correct priorities use a separately branded stale nonce.
This demonstrates that reproducing the prescribed algorithm is insufficient
when it is bound to a different client request.

### 14 — Reuse one draw (negative)

One nonce-bound draw from site 0 supplies all eight priorities. The keys tie and
the specified tie rule selects record 0, but sites 1 through 7 are absent from
the proof graph. Distinct CERTRNG site identities make the reuse detectable.

### 15 — Drop candidate 5 (negative)

Candidate 5 is assigned candidate 4's key. The fixed comparison still executes,
but candidate 5's random draw and nonce XOR never reach the result. This models
a server trying to protect a suspicious record by excluding it from the audit.

### 16 — Use draws at the wrong selection points (negative)

Candidates 2 and 5 receive draws from sites 5 and 2 respectively. Every required
site exists, but the candidate-to-site relationships in the argmin graph differ
from the client's graph, and disclosure is rejected.

### 17 — Return a precomputed selection (negative)

The server runs the complete algorithm using the old branded nonce and returns
that stored result for the fresh request. The stale brand proves that the result
was not computed for this request. This case is deliberately separate from test
13 even though both demonstrate request binding: test 13 substitutes an old input
during service, while test 17 models replaying a previously stored output.

## Running the matrix

```sh
make TARGET=mojov clean build test
```

The test target treats a successful malicious run as a failure: cases 10–17 must
trap during datagrant testing, while the honest run and the two explanatory
controls must exit normally. Except for case 2's deliberately unsafe experiment,
the server never receives a disclosed winner before commitment.
