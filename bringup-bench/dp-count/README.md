# Verifiable Differentially Private Count

This demonstrator models a trusted client requesting a count from an untrusted
server. The input count and request identity arrive encrypted. Mojo-V protected
code adds **centered binomial noise**—the difference of two independent
`Binomial(8, 1/2)` variables—and returns an encrypted answer. The server cannot
observe the random draw or released count before the client validates its
dataflow receipt and decrypts it.

The mechanism uses 16 distinct CERTRNG sites. Every random bit is mixed with the
branded request value, and the client's datagrant names the exact approved
arithmetic graph. This benchmark demonstrates certification of the mechanism's
implementation; selecting privacy parameters and accounting for repeated
authorized releases remain application-level responsibilities.

## Test cases

The simulator argument `--mojov-arg=N` selects a standalone case:

- **0 — honest mechanism:** Executes the exact 16-draw centered-binomial graph.
- **1 — unused draw:** Shows that unrelated randomness outside the result's
  ancestry does not alter its receipt.
- **10 — omit noise:** Returns the true count directly and is rejected.
- **11 — reduce magnitude:** Uses only half the approved trials and is rejected.
- **12 — replace the generator:** Substitutes software `rand()` and is rejected
  because ordinary loads do not have CERTRNG provenance.
- **13 — favorable resampling:** Computes another candidate and selects the
  larger encrypted answer. The selection and extra sites change the receipt,
  preventing repeated sampling from silently biasing a release.
- **14 — replay:** Runs the approved mechanism against a stale branded request;
  request binding makes its receipt invalid for the current client request.

Cases 10–14 intentionally terminate with a Mojo-V security exception when the
honest datagrant is tested. In every ordinary case the result remains encrypted
until after this test, so the server has no outcome-classification oracle.

## Run

```sh
make TARGET=mojov clean build test
```
