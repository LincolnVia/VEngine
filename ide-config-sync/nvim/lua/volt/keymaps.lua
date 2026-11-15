local map = vim.keymap.set

map("n", "<Esc>", "<cmd>nohlsearch<CR>")
map("n", "<leader>w", "<cmd>write<CR>", { desc = "Save file" })
map("n", "<leader>q", "<cmd>quit<CR>", { desc = "Quit window" })
map("n", "<leader>lR", "<cmd>Telescope find_files<CR>", { desc = "Open project files" })

local function create_unity_script(kind)
	local templates = {
		MonoBehaviour = function(class_name)
			return {
				"using UnityEngine;",
				"",
				"public class " .. class_name .. " : MonoBehaviour",
				"{",
				"    private void Start()",
				"    {",
				"    }",
				"",
				"    private void Update()",
				"    {",
				"    }",
				"}",
			}
		end,
		ScriptableObject = function(class_name)
			return {
				"using UnityEngine;",
				"",
				'[CreateAssetMenu(menuName = "' .. class_name .. '", fileName = "' .. class_name .. '")]',
				"public class " .. class_name .. " : ScriptableObject",
				"{",
				"}",
			}
		end,
		EditorWindow = function(class_name)
			return {
				"using UnityEditor;",
				"",
				"public class " .. class_name .. " : EditorWindow",
				"{",
				'    [MenuItem("Window/' .. class_name .. '")]',
				"    public static void ShowWindow()",
				"    {",
				'        GetWindow<' .. class_name .. '>("' .. class_name .. '");',
				"    }",
				"}",
			}
		end,
	}

	local function selected_directory()
		local ok, api = pcall(require, "nvim-tree.api")
		if ok then
			local node = api.tree.get_node_under_cursor()
			if node and node.absolute_path and node.absolute_path ~= "" then
				local path = node.absolute_path
				if node.type ~= "directory" then
					path = vim.fn.fnamemodify(path, ":h")
				end
				return path
			end
		end
		return vim.fn.getcwd()
	end

	vim.ui.input({ prompt = "New " .. kind .. " (name or relative path): " }, function(input)
		if not input or input:match("^%s*$") then
			return
		end

		input = input:gsub("\\", "/"):gsub("%.cs$", "")
		local relative_dir, class_name = input:match("^(.*)/([^/]+)$")
		class_name = class_name or input
		if not class_name:match("^[A-Za-z_][A-Za-z0-9_]*$") then
			vim.notify("Use a valid C# class name (letters, digits, and underscores)", vim.log.levels.ERROR)
			return
		end

		local base_dir = selected_directory()
		local output_dir = relative_dir and (base_dir .. "/" .. relative_dir) or base_dir
		local path = output_dir .. "/" .. class_name .. ".cs"
		if vim.fn.filereadable(path) == 1 then
			vim.notify("File already exists: " .. path, vim.log.levels.ERROR)
			return
		end
		local escapes_project = false
		if relative_dir then
			for segment in relative_dir:gmatch("[^/]+") do
				if segment == ".." then
					escapes_project = true
					break
				end
			end
		end
		if relative_dir and (relative_dir:match("^/") or escapes_project) then
			vim.notify("Enter a relative path inside the current project", vim.log.levels.ERROR)
			return
		end
		if relative_dir then
			if vim.fn.mkdir(output_dir, "p") == 0 and vim.fn.isdirectory(output_dir) == 0 then
				vim.notify("Could not create directory: " .. output_dir, vim.log.levels.ERROR)
				return
			end
		end

		local ok, result = pcall(vim.fn.writefile, templates[kind](class_name), path)
		if not ok or result ~= 0 then
			local err = ok and ("writefile returned " .. tostring(result)) or result
			vim.notify("Could not create " .. path .. ": " .. tostring(err), vim.log.levels.ERROR)
			return
		end
		vim.cmd.edit(vim.fn.fnameescape(path))
		vim.notify("Created " .. kind .. ": " .. path, vim.log.levels.INFO)
	end)
end

map("n", "<leader>uc", function() create_unity_script("MonoBehaviour") end, { desc = "Create MonoBehaviour script" })
map("n", "<leader>us", function() create_unity_script("ScriptableObject") end, { desc = "Create ScriptableObject script" })
map("n", "<leader>ue", function() create_unity_script("EditorWindow") end, { desc = "Create EditorWindow script" })
map("n", "<leader>uf", "<cmd>Telescope find_files find_command=rg,--files,-g,*.cs<CR>", { desc = "Find C# scripts" })

-- Window controls (<leader>w...). The existing <leader>w save mapping remains
-- available; these longer mappings add the window actions under the same prefix.
map("n", "<leader>wv", "<cmd>vsplit<CR>", { desc = "Split window vertically" })
map("n", "<leader>wh", "<cmd>split<CR>", { desc = "Split window horizontally" })
map("n", "<leader>wc", "<cmd>close<CR>", { desc = "Close current window" })
map("n", "<leader>wo", "<cmd>only<CR>", { desc = "Keep only current window" })
map("n", "<leader>w=", "<C-w>=", { desc = "Equalize window sizes" })
map("n", "<leader>wH", "<C-w>H", { desc = "Move window to far left" })
map("n", "<leader>wJ", "<C-w>J", { desc = "Move window to bottom" })
map("n", "<leader>wK", "<C-w>K", { desc = "Move window to top" })
map("n", "<leader>wL", "<C-w>L", { desc = "Move window to far right" })

local function commit_and_push()
	local root = vim.fn.system({ "git", "rev-parse", "--show-toplevel" }):gsub("%s+$", "")
	if vim.v.shell_error ~= 0 or root == "" then
		vim.notify("Current directory is not inside a Git repository", vim.log.levels.ERROR)
		return
	end

	vim.ui.input({ prompt = "Commit message: " }, function(message)
		if not message or message:match("^%s*$") then
			vim.notify("Commit cancelled: message is required", vim.log.levels.WARN)
			return
		end

		local function git(args)
			local command = { "git", "-C", root }
			vim.list_extend(command, args)
			local output = vim.fn.system(command)
			if vim.v.shell_error ~= 0 then
				vim.notify(output ~= "" and output or "Git command failed", vim.log.levels.ERROR)
				return false
			end
			return true
		end

		if not git({ "add", "-A" }) then
			return
		end
		if not git({ "commit", "-m", message }) then
			return
		end

		local branch = vim.fn.system({ "git", "-C", root, "branch", "--show-current" }):gsub("%s+$", "")
		if vim.v.shell_error ~= 0 or branch == "" then
			vim.notify("Commit succeeded, but no current branch is available to push", vim.log.levels.ERROR)
			return
		end

		if git({ "push", "-u", "origin", branch }) then
			vim.notify("Committed and pushed " .. branch, vim.log.levels.INFO)
		end
	end)
end

map("n", "<leader>gp", commit_and_push, { desc = "Stage, commit, and push project" })

local function commit_and_merge_to_main()
	local root = vim.fn.system({ "git", "rev-parse", "--show-toplevel" }):gsub("%s+$", "")
	if vim.v.shell_error ~= 0 or root == "" then
		vim.notify("Current directory is not inside a Git repository", vim.log.levels.ERROR)
		return
	end

	local function git(args)
		local command = { "git", "-C", root }
		vim.list_extend(command, args)
		local output = vim.fn.system(command)
		if vim.v.shell_error ~= 0 then
			vim.notify(output ~= "" and output or "Git command failed", vim.log.levels.ERROR)
			return false
		end
		return true
	end

	local branch = vim.fn.system({ "git", "-C", root, "branch", "--show-current" }):gsub("%s+$", "")
	if vim.v.shell_error ~= 0 or branch == "" then
		vim.notify("Could not determine the current branch", vim.log.levels.ERROR)
		return
	end
	if branch == "main" then
		vim.notify("Switch to a feature branch before merging to main", vim.log.levels.WARN)
		return
	end

	vim.ui.input({ prompt = "Commit message: " }, function(message)
		if not message or message:match("^%s*$") then
			vim.notify("Commit and merge cancelled: message is required", vim.log.levels.WARN)
			return
		end

		if not git({ "add", "-A" }) then
			return
		end
		if not git({ "commit", "-m", message }) then
			return
		end
		if not git({ "switch", "main" }) then
			return
		end
		if not git({ "merge", "--no-edit", branch }) then
			vim.notify("Merge failed; resolve the merge on main before continuing", vim.log.levels.ERROR)
			return
		end
		if git({ "switch", branch }) then
			vim.notify("Committed " .. branch .. " and merged it into main", vim.log.levels.INFO)
		end
	end)
end

map("n", "<leader>gm", commit_and_merge_to_main, { desc = "Stage, commit, and merge into main" })

-- QWERTY window navigation.
map("n", "<C-h>", "<C-w><C-h>", { desc = "Move to left window" })
map("n", "<C-j>", "<C-w><C-j>", { desc = "Move to lower window" })
map("n", "<C-k>", "<C-w><C-k>", { desc = "Move to upper window" })
map("n", "<C-l>", "<C-w><C-l>", { desc = "Move to right window" })

map("v", "J", ":m '>+1<CR>gv=gv", { desc = "Move selection down" })
map("v", "K", ":m '<-2<CR>gv=gv", { desc = "Move selection up" })
map("n", "J", "mzJ`z")
map("n", "<C-d>", "<C-d>zz")
map("n", "<C-u>", "<C-u>zz")
map("n", "n", "nzzzv")
map("n", "N", "Nzzzv")

map("x", "<leader>p", [['_dP]], { desc = "Paste without replacing register" })
map({ "n", "v" }, "<leader>y", [["+y]], { desc = "Yank to system clipboard" })
map("n", "<leader>Y", [["+Y]], { desc = "Yank line to system clipboard" })
map({ "n", "v" }, "<leader>d", [['_d]], { desc = "Delete without replacing register" })

map("n", "[q", "<cmd>cprevious<CR>zz", { desc = "Previous quickfix item" })
map("n", "]q", "<cmd>cnext<CR>zz", { desc = "Next quickfix item" })
map("n", "[l", "<cmd>lprevious<CR>zz", { desc = "Previous location item" })
map("n", "]l", "<cmd>lnext<CR>zz", { desc = "Next location item" })

map("n", "<leader>sr", [[:%s/\<<C-r><C-w>\>/<C-r><C-w>/gI<Left><Left><Left>]], { desc = "Replace word under cursor" })
map("t", "<Esc><Esc>", "<C-\\><C-n>", { desc = "Exit terminal mode" })
local function generate_csharp_get_set()
	if vim.bo.filetype ~= "cs" then
		vim.notify("C# getter/setter generator works in C# files", vim.log.levels.WARN)
		return
	end

	local row = vim.api.nvim_win_get_cursor(0)[1]
	local line = vim.api.nvim_get_current_line()
	local indent, access, declaration
	for _, visibility in ipairs({ "public", "private", "protected", "internal" }) do
		indent, declaration = line:match("^(%s*)" .. visibility .. "%s+(.+);%s*$")
		if declaration then
			access = visibility
			break
		end
	end

	local field_name, field_type, modifiers
	if declaration then
		local lhs, before_name
		lhs = declaration:match("^(.-)%s*=.+$") or declaration
		before_name, field_name = lhs:match("^(.-)%s+(@?[%a_][%w_]*)%s*$")
		if before_name then
			local modifier_names = { static = true, new = true, volatile = true, readonly = true, const = true, unsafe = true, required = true }
			modifiers = ""
			field_type = before_name
			while true do
				local first, remainder = field_type:match("^([%a_][%w_]*)%s+(.+)$")
				if not first or not modifier_names[first] then
					break
				end
				modifiers = modifiers .. first .. " "
				field_type = remainder
			end
			field_type = field_type:gsub("%s+$", "")
		end
	end
	if not field_name or not field_type or field_type == "" then
		vim.notify("Place the cursor on a simple C# field declaration", vim.log.levels.WARN)
		return
	end
	if modifiers:match("%f[%w]const%f[%W]") or modifiers:match("%f[%w]readonly%f[%W]") then
		vim.notify("Cannot generate a set accessor for const or readonly fields", vim.log.levels.WARN)
		return
	end

	local property_name = field_name:gsub("^@", ""):gsub("^m_", ""):gsub("^_", "")
	property_name = property_name:sub(1, 1):upper() .. property_name:sub(2)
	local static = modifiers:match("%f[%w]static%f[%W]") and "static " or ""
	local property_access = access == "private" and "public" or access
	local backing_field = field_name:gsub("^@", "")
	local property = string.format(
		"%s%s %s%s %s { get => %s; set => %s = value; }",
		indent,
		property_access,
		static,
		field_type,
		property_name,
		backing_field,
		backing_field
	)
	vim.api.nvim_buf_set_lines(0, row, row, false, { property })
	vim.notify("Generated " .. property_access .. " property " .. property_name, vim.log.levels.INFO)
end

map("n", "<leader>lg", generate_csharp_get_set, { desc = "Generate C# get/set property" })

local function generate_cpp_get_set()
	-- Get the word under the cursor (the variable name)
	local var_name = vim.fn.expand("<cword>")
	if var_name == "" then
		return
	end

	-- Clean up common prefixes like m_ or trailing underscores
	local clean_name = var_name:gsub("^m_", ""):gsub("_$", "")
	-- Capitalize first letter for CamelCase methods (e.g., GetValue)
	local cap_name = clean_name:sub(1, 1):upper() .. clean_name:sub(2)

	-- Ask the user for the variable type (or default to int/auto)
	vim.ui.input({ prompt = "Type for " .. var_name .. ": ", default = "int" }, function(type)
		if not type or type == "" then
			return
		end

		-- Generate C++ boilerplate strings
		local getter = string.format("    %s get%s() const { return %s; }", type, cap_name, var_name)
		local setter = string.format("    void set%s(%s val) { %s = val; }", cap_name, type, var_name)

		-- Insert right below the current line
		local row, _ = unpack(vim.api.nvim_win_get_cursor(0))
		vim.api.nvim_buf_set_lines(0, row, row, false, { getter, setter })
	end)
end

-- Keymap to trigger the generator: press <leader>gs on a variable
vim.keymap.set("n", "<leader>lf", generate_cpp_get_set, { desc = "Generate C++ Getter/Setter" })
