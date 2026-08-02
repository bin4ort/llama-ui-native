// Meta parser for `exec_shell_command` tool calls. Surfaces the
// command text from args `command` / `cmd` / `shell_command` aliases.
// The exit-status and error parsing live in their own utilities
// (`parse-exec-shell-status.ts` / `parse-exec-shell-error.ts`) - this
// file only deals with what's strictly about *calling* the tool, since
// the error / exit status elide from call-section to result-section.
import { BuiltInTool } from '$lib/enums';
import { parseToolArgs } from './_shared';
export function parseExecShellCommandMeta(section) {
    const args = parseToolArgs(BuiltInTool.EXEC_SHELL_COMMAND, section);
    if (!args)
        return null;
    const commandRaw = args.command ?? args.cmd ?? args.shell_command;
    if (typeof commandRaw !== 'string' || !commandRaw)
        return null;
    return { command: commandRaw };
}
