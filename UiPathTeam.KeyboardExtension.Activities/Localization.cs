using System;
using System.ComponentModel;

namespace UiPathTeam.KeyboardExtension.Activities
{
    [AttributeUsage(AttributeTargets.Property)]
    public sealed class LocalizedCategoryAttribute : CategoryAttribute
    {
        public LocalizedCategoryAttribute(string category)
            : base(category)
        {
        }

        protected override string GetLocalizedString(string value)
        {
            return Resource.ResourceManager.GetString(value) ?? base.GetLocalizedString(value);
        }
    }

    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Class)]
    public sealed class LocalizedDisplayNameAttribute : DisplayNameAttribute
    {
        public LocalizedDisplayNameAttribute(string displayName)
            : base(displayName)
        {
        }

        public override string DisplayName
        {
            get
            {
                return Resource.ResourceManager.GetString(DisplayNameValue) ?? base.DisplayName;
            }
        }
    }

    [AttributeUsage(AttributeTargets.Property | AttributeTargets.Class)]
    public sealed class LocalizedDescriptionAttribute : DescriptionAttribute
    {
        public LocalizedDescriptionAttribute(string displayName)
            : base(displayName)
        {
        }

        public override string Description
        {
            get
            {
                return Resource.ResourceManager.GetString(DescriptionValue) ?? base.Description;
            }
        }
    }
}