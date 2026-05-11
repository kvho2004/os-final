# os-final

*Spec (Copied from Canvas Assignment Description)*

# Part 1 - The Basic Scenario
Consider a house belonging to the old cat lady, who has several cats (C represents the number of cats). Each cat is fed from one of the several bowls (B represents the number of bowls). The cat food lures mice (M) that are also hungry. All of them live together in a symbiosis under the following rules:

1. The bowls have an infinite amount of food.
2. Each cat eventually gets hungry, and when it does, it looks for an unoccupied bowl (specifically a cat-free bowl). 
3. If all bowls are occupied by other cats, it has to wait until one becomes available.
4. If at least one bowl is available, it can start feeding itself.
5. Every bowl can be occupied by a single cat at a time.
6. Each cat feeds itself for a limited time.
7. Each mouse eventually gets hungry, but it has to wait until all the cats are away (no cat is feeding at a time).
8. If there are no cats feeding at a time, the mouse can pick a bowl and feed itself.
9. If at least one cat is feeding, the mouse has to wait until no cats are in sight.
10. If all bowls are occupied by other mice, the mouse has to wait until one of them finishes feeding.
11. Each mouse feeds itself until it is interrupted by a cat arriving for food.
12. When a cat arrives at the bowls, all the feeding mice have to stop and run away.
13. Cats must leave some time for mice to feed (otherwise you cannot test, whether the symbiosis works).


# The task specification
(group) Analyze the scenario and choose suitable synchronization techniques.
(group) Create two implementations of the given scenario with:
  1. threads
  2. processes
     
(group) Try various configurations of the scenario parameters to test whether the symbiosis works well and is never stuck:
  1. the number of bowls (B)
  2. the number of cats (C)
  3. the number of mice (M)
  4. the duration of a cat feeding (F in time units, could also be a random interval)
  5. the duration of a cat not being hungry (N in time units, could also be a random interval)

(group) Discuss the critical synchronization points and how you solved them.

(group) Compare both implementation approaches (threads and processes) and discuss the advantages and disadvantages of each. As a result, choose the best approach to be used in the second part of the scenario.

(individual) Reflect your individual performance and your team members' performance.

**Deliverables**
(group): One zip file per group containing:
  1. The source code of the implementation of the scenario with threads.
  2. The source code of the implementation of the scenario with processes.
  3. The document containing all non-coding parts of the group project:
  4. The discussion about the synchronization points and the justification of the used techniques.
  5. The discussion about both implemented approaches (pros and cons, similarities and differences).
  6. The worklog showing the project participation of all group members.
     
(individual): One document in the form of a PDF with the reflection.

**AI policy**
The use of AI is not allowed for this project. The students are obligated to keep a history of their work in the Git repository. Every significant change in the project code or the textual document should be pushed and archived in Git.

Strong recommendation: Create a project (or prjects) on a class GitLab server and keep everything updated between all project participants.

# Part 2 
Part 1 - The Basic Scenario Consider a house belonging to the old cat lady, who has several cats (C represents the number of cats). Each cat is fed from one of the several bowls (B represents the number of bowls). The cat food lures mice (M) that are also hungry. All of them live together in a symbiosis under the following rules:

The bowls have an infinite amount of food.

Each cat eventually gets hungry, and when it does, it looks for an unoccupied bowl (specifically a cat-free bowl).

If all bowls are occupied by other cats, it has to wait until one becomes available.

If at least one bowl is available, it can start feeding itself.

Every bowl can be occupied by a single cat at a time.

Each cat feeds itself for a limited time.

Each mouse eventually gets hungry, but it has to wait until all the cats are away (no cat is feeding at a time).

If there are no cats feeding at a time, the mouse can pick a bowl and feed itself.

If at least one cat is feeding, the mouse has to wait until no cats are in sight.

If all bowls are occupied by other mice, the mouse has to wait until one of them finishes feeding.

Each mouse feeds itself until it is interrupted by a cat arriving for food.

When a cat arrives at the bowls, all the feeding mice have to stop and run away.

Cats must leave some time for mice to feed (otherwise you cannot test, whether the symbiosis works). The task specification

(group) Analyze the scenario and choose suitable synchronization techniques.

(group) Create two implementations of the given scenario with:

threads

processes

(group) Try various configurations of the scenario parameters to test whether the symbiosis works well and is never stuck:

the number of bowls (B)

the number of cats (C)

the number of mice (M)

the duration of a cat feeding (F in time units, could also be a random interval)

the duration of a cat not being hungry (N in time units, could also be a random interval)

(group) Discuss the critical synchronization points and how you solved them.

(group) Compare both implementation approaches (threads and processes) and discuss the advantages and disadvantages of each. As a result, choose the best approach to be used in the second part of the scenario.

(individual) Reflect your individual performance and your team members' performance. Deliverables

(group): One zip file per group containing: 1. The source code of the implementation of the scenario with threads. 2. The source code of the implementation of the scenario with processes. 3. The document containing all non-coding parts of the group project:

The discussion about the synchronization points and the justification of the used techniques.

The discussion about both implemented approaches (pros and cons, similarities and differences).

The worklog showing the project participation of all group members.

(individual): One document in the form of a PDF with the reflection. i want you to break down this assigment into 4 diffrent commits so each memeber can do it along side the individual part and get 100% on the assigment for each member

vikturek@gmail.com