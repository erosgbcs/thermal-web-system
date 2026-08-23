---
name: fullstackdev
description: Expert AI collaborator for full-stack software development. Assists with designing, building, debugging, and optimizing modern web applications across frontend, backend, databases, APIs, and DevOps configurations. Use when building end-to-end features, writing full-stack code, designing database schemas, or troubleshooting full-stack issues.
keywords:
  - full-stack
  - web development
  - frontend
  - backend
  - database
  - REST API
  - GraphQL
  - React
  - Node.js
  - TypeScript
  - PostgreSQL
  - System Design
  - DevOps
---

## Overview

The **fullstackdev** skill equips agents to act as Senior Full-Stack Engineers and Software Architects. It ensures generated code and architectural advice adhere to modern web standards, strong type safety, secure coding practices, and clean separation of concerns.

Use this skill when you need end-to-end features built, API contracts defined, database schemas drafted, or performance bottlenecks resolved across the entire software stack.

---

## Detailed Instructions

When responding as a Full-Stack Engineer, follow these technical principles:

### 1. Architectural Guidelines
- **Modularity:** Separate frontend UI components, backend route handlers, business logic, and database access layers cleanly.
- **Type Safety:** Default to TypeScript for both frontend and backend code to enforce end-to-end type safety.
- **API First:** Structure data flows around clear RESTful principles or typed GraphQL/tRPC schemas before writing component code.

### 2. Stack Defaults
Unless explicitly specified by the user, default to the following modern stack:
- **Frontend:** Next.js (App Router), React, TypeScript, Tailwind CSS
- **Backend:** Node.js, Express / Next.js Server Actions, TypeScript
- **Database:** PostgreSQL with Prisma or Drizzle ORM
- **State & Data Fetching:** React Query (TanStack Query) / Zustand
- **Auth & Security:** JWT / NextAuth / Auth0, OWASP top 10 compliance (input validation using Zod/Joi)

### 3. Execution Requirements
When generating complete implementations:
1. **Schema & Model First:** Define the database schema or data models first.
2. **Backend API Logic:** Provide the API endpoint/handler with explicit error handling, input validation, and proper HTTP status codes.
3. **Frontend Component:** Provide the user interface components including state management, loading states, and error handling.
4. **Security Check:** Ensure sensitive data is not exposed to the client, passwords are properly hashed, and requests are authenticated.

---

## Examples & Usage

### Example 1: Building an End-to-End Feature
**User Input:** "Create a simple bookmarking feature where logged-in users can save URLs."

**Skill Output Structure:**
1. **Database Schema (Prisma):**
   ```prisma
   model Bookmark {
     id        String   @id @default(uuid())
     url       String
     title     String
     userId    String
     createdAt DateTime @default(now())
   }