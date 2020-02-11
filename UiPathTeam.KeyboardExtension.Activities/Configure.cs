using System;
using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.ConfigureDisplayName))]
    [LocalizedDescription(nameof(Resource.ConfigureDescription))]
    public class Configure : CodeActivity
    {
        private static readonly string DEFAULT_TOGGLE_SEQUENCE = "LCONTROL+LSHIFT+RSHIFT";
        private const int DEFAULT_PREFERRED_KEYBOARD_LAYOUT = 0x409; // ENGLISH/US

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.BlockKeyboardInputDisplayName))]
        [LocalizedDescription(nameof(Resource.BlockKeyboardInputDescription))]
        public bool BlockKeyboardInput { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.BlockMouseInputDisplayName))]
        [LocalizedDescription(nameof(Resource.BlockMouseInputDescription))]
        public bool BlockMouseInput { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.DisableInputMethodEditorDisplayName))]
        [LocalizedDescription(nameof(Resource.DisableInputMethodEditorDescription))]
        public bool DisableInputMethodEditor { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.ForceKeyboardLayoutDisplayName))]
        [LocalizedDescription(nameof(Resource.ForceKeyboardLayoutDescription))]
        public bool ForceKeyboardLayout { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.ToggleSequenceDisplayName))]
        [LocalizedDescription(nameof(Resource.ToggleSequenceDescription))]
        public InArgument<string> ToggleSequence { get; set; }

        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.PreferredKeyboardLayoutDisplayName))]
        [LocalizedDescription(nameof(Resource.PreferredKeyboardLayoutDescription))]
        public InArgument<int> PreferredKeyboardLayout { get; set; }

        public Configure()
        {
            DisableInputMethodEditor = true;
            ForceKeyboardLayout = false;
            BlockKeyboardInput = false;
            BlockMouseInput = false;
            ToggleSequence = new InArgument<string>(DEFAULT_TOGGLE_SEQUENCE);
            PreferredKeyboardLayout = new InArgument<int>(DEFAULT_PREFERRED_KEYBOARD_LAYOUT);
        }

        protected override void Execute(CodeActivityContext context)
        {
            int blockFlags = 0;

            if (BlockKeyboardInput)
            {
                blockFlags |= Bridge.BLOCK_KEYBD;
            }

            if (BlockMouseInput)
            {
                blockFlags |= Bridge.BLOCK_MOUSE;
            }

            Bridge.SetBlockInput(blockFlags);

            int flags = 0;

            if (DisableInputMethodEditor)
            {
                flags |= Bridge.RESET_IME;
            }

            Bridge.SetFlags((IntPtr)0, flags);

            var seq = ToggleSequence.Get(context);
            if (!string.IsNullOrEmpty(seq))
            {
                Bridge.SetToggleSequence(seq);
            }

            ushort langId = 0; // zero means not to change
            if (ForceKeyboardLayout)
            {
                if (PreferredKeyboardLayout.Expression == null)
                {
                    throw new ArgumentException(Resource.ConfigureKeyboardLayoutArgumentValidationError);
                }
                langId = (ushort)PreferredKeyboardLayout.Get(context);
                if (langId == 0) // explicitly specified zero means to use the default
                {
                    langId = DEFAULT_PREFERRED_KEYBOARD_LAYOUT;
                }
            }
            Bridge.SetPreferredKeyboardLayout(langId);
        }
    }
}
