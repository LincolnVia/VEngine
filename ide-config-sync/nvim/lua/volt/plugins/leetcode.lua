-- Add this plugin spec to ~/.config/nvim/lua/volt/plugins/leetcode.lua.
return {
	{
		"kawre/leetcode.nvim",
		cmd = "Leet",
		dependencies = {
			"nvim-lua/plenary.nvim",
			"MunifTanjim/nui.nvim",
			"nvim-telescope/telescope.nvim",
			{
				"3rd/image.nvim",
				version = ">=1.4.0",
				opts = {
					backend = "kitty",
					processor = "magick_cli",
				},
			},
		},
		opts = {
			lang = "cpp",
			image_support = true,
			injector = {
				["cpp"] = {
					imports = function(default_imports)
						vim.list_extend(default_imports, {
							"struct ListNode {",
							"    int val;",
							"    ListNode *next;",
							"    ListNode() : val(0), next(nullptr) {}",
							"    ListNode(int x) : val(x), next(nullptr) {}",
							"    ListNode(int x, ListNode *n) : val(x), next(n) {}",
							"};",
							"struct TreeNode {",
							"    int val;",
							"    TreeNode *left;",
							"    TreeNode *right;",
							"    TreeNode() : val(0), left(nullptr), right(nullptr) {}",
							"    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}",
							"    TreeNode(int x, TreeNode *l, TreeNode *r) : val(x), left(l), right(r) {}",
							"};",
						})
						return default_imports
					end,
				},
			},
			plugins = { non_standalone = true },
			picker = { provider = "telescope" },
			storage = {
				home = vim.fn.stdpath("data") .. "/leetcode",
				cache = vim.fn.stdpath("cache") .. "/leetcode",
			},
		},
	},
}
