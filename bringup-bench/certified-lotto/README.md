# Certified Lottery and Tie-Breaker

This CERTRNG demonstrator models a trusted client asking an untrusted server to
choose a winner from eight encrypted eligible participants. The encrypted
scores are `12, 19, 19, 7, 19, 15, 15, 3`, deliberately creating a three-way
tie for first and a second two-way tie. Mojo-V protected code gives every
participant a distinct certified random priority, binds it to the client's
encrypted request nonce, and selects the highest score and then the lowest
priority without revealing either scores or intermediate draws.

The client's datagrant names the exact fixed tournament: all eight encrypted
participants, all eight CERTRNG sites, all nonce mixes, and all seven selection
steps. Consequently, removing an eligible participant or changing the tie
breaker produces a different receipt. The server returns the encrypted winner
and receipt first; only after the client accepts that pair as the committed
result is the winner decrypted. This ordering matters: certified randomness
does not prevent a server that sees each result from retrying until it likes the
winner.

This benchmark certifies a randomized selection mechanism, rather than a
differential-privacy mechanism. It complements `diffpriv-count`, which uses the
same request-bound CERTRNG and encrypted-return pattern to certify DP noise.

## Test cases

The simulator argument `--mojov-arg=N` selects one standalone case:

- **0 — honest tied draw:** includes all eight participants and resolves the
  three-way winning tie with request-bound certified priorities.
- **1 — unused draw:** proves unrelated random work is not in the result receipt.
- **2 — disclose before commitment:** repeatedly discloses otherwise valid
  draws until participant 4 wins, demonstrating the grinding hazard.
- **10 — omit participant:** substitutes participant 1 for participant 4.
- **11 — software RNG:** replaces participant 2's CERTRNG draw with `rand()`.
- **12 — skip nonce:** leaves participant 1's draw unbound to this request.
- **13 — stale request:** uses a nonce branded for an earlier request.
- **14 — reuse draw:** gives every participant the same random priority.
- **15 — wrong sites:** swaps the random sites assigned to participants 1 and 4.
- **16 — deterministic tie-break:** favors the first tied participant without
  using the approved random tournament.

Cases 10–16 intentionally terminate with a Mojo-V security exception when the
honest datagrant is tested. Case 2 is intentionally unsafe and exits normally
to make the disclosure-before-commitment failure mode visible.

## Run

```sh
make TARGET=mojov clean build test
```
