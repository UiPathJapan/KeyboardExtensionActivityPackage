using System.Activities;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [LocalizedDisplayName(nameof(Resource.GetStateDisplayName))]
    [LocalizedDescription(nameof(Resource.GetStateDescription))]
    public class GetState : CodeActivity
    {
        [LocalizedCategory(nameof(Resource.InputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateWindowHandleDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateWindowHandleDescription))]
        [RequiredArgument]
        public InArgument<object> WindowHandle { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateKeyboardLayoutDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateKeyboardLayoutDescription))]
        public OutArgument<int> KeyboardLayout { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateKeyboardOpenCloseDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateKeyboardOpenCloseDescription))]
        public OutArgument<bool> KeyboardOpenClose { get; set; }

        [LocalizedCategory(nameof(Resource.OutputCategory))]
        [LocalizedDisplayName(nameof(Resource.GetStateConversionModeDisplayName))]
        [LocalizedDescription(nameof(Resource.GetStateConversionModeDescription))]
        public OutArgument<int> ConversionMode { get; set; }

        protected override void Execute(CodeActivityContext context)
        {
            var value = Bridge.GetState(Bridge.ToWin32Handle(WindowHandle.Get(context)));
            if (KeyboardLayout != null)
            {
                KeyboardLayout.Set(context, (int)((value >> 0) & 0xffff));
            }
            if (KeyboardOpenClose != null)
            {
                KeyboardOpenClose.Set(context, ((value >> 31) & 1) == 1);
            }
            if (ConversionMode != null)
            {
                ConversionMode.Set(context, (int)((value >> 16) & 0x7fff));
            }
        }
    }
}
